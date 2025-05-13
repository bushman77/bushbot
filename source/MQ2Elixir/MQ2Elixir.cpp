#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>
#include <cstdarg>
#include <chrono>
#include "../MQ2Plugin.h"
#include "MQ2Elixir.h"

#pragma comment(lib, "ws2_32.lib")

PreSetup("MQ2Elixir");

// ──────────────────────────────────────────────────────────────
// Config & globals
// ──────────────────────────────────────────────────────────────
constexpr int  BUFFER_SIZE = 4096;
constexpr char ELIXIR_HOST[] = "127.0.0.1";
constexpr int  ELIXIR_PORT = 4000;
constexpr int  STANDSTATE_COMBAT = 100;
constexpr auto HEARTBEAT_INTERVAL = std::chrono::seconds(1);

static std::deque<std::string> g_outQueue;
static std::deque<std::string> g_inQueue;
static std::mutex              g_outMu, g_inMu;
static std::condition_variable g_outCv;
static std::thread             g_socketThread;
static bool                    g_registered{ false };
static SOCKET                  g_socket = INVALID_SOCKET;
static std::atomic<bool>       g_running{ false };
// ──────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────
void SetEQWindowTitle() {
	HWND hwnd = GetEQWindowHandle();
	if (!hwnd) return;

	auto* pChar = GetCharInfo();
	if (!pChar || !pChar->pSpawn) return;

	// Build the new title
	std::string title = "EverQuest - ";
	title.append(pChar->Name);

	// Set it
	SetWindowTextA(hwnd, title.c_str());
}
bool IsActiveClient() {
	return GetForegroundWindow() == GetEQWindowHandle();
}
void safe_close_socket() {
	if (g_socket != INVALID_SOCKET) {
		closesocket(g_socket);
		g_socket = INVALID_SOCKET;
	}
}
void EnqueueCommand(const std::string& cmd) {
	WriteChatf("Firing EnqueueCommand(cmd)");
	{
		std::lock_guard<std::mutex> lk(g_outMu);
		g_outQueue.push_back(cmd + "\n");
	}
	//g_outCv.notify_one();
	 // 🔔 wake everyone waiting
	g_outCv.notify_all();
}
static void FlushOutgoing() {
	// 🔍 STEP 1: Are we even getting in here?
	WriteChatf("Firing FlushOutgoing()");
	//WriteChatf("[MQ2Elixir] → FlushOutgoing() called; queue size before swap: %zu", g_outQueue.size());

	std::deque<std::string> local;
	{
		std::lock_guard<std::mutex> lk(g_outMu);
		local.swap(g_outQueue);
	}
	for (auto& msg : local) {
		if (g_socket != INVALID_SOCKET) {
			// 🔴 DEBUG: show what we're about to send
			WriteChatf("[MQ2Elixir] Sending → %s", msg.c_str());
			int sent = send(g_socket, msg.c_str(), static_cast<int>(msg.size()), 0);
			WriteChatf("[MQ2Elixir] send() returned: %d", sent);
		}
	}
}
std::string BuildCharacterJson() {
	auto* pChar = GetCharInfo();
	if (!pChar || !pChar->pSpawn) {
		WriteChatf("BuildCharacterJson: no character yet");
		return "{}";
		
	}
	
	// (now we know pChar and pChar->pSpawn are valid)
	bool active = IsActiveClient();
	bool combat = pChar->pSpawn->StandState == STANDSTATE_COMBAT;
	std::string js;
	std::string name = pChar->pSpawn->Name;

	js.reserve(256);  // optional: avoid reallocs

	js.append("{\"character\":{");
	js.append("\"name\":\"").append(name).append("\",");
	js.append("\"class_id\":").append(std::to_string(pChar->pSpawn->mActorClient.Class)).append(",");
	js.append("\"level\":").append(std::to_string(pChar->pSpawn->Level)).append(",");
	js.append("\"hp\":").append(std::to_string(pChar->pSpawn->HPCurrent)).append(",");
	js.append("\"max_hp\":").append(std::to_string(pChar->pSpawn->HPMax)).append(",");
	js.append("\"mp\":").append(std::to_string(pChar->pSpawn->ManaCurrent)).append(",");
	js.append("\"max_mp\":").append(std::to_string(pChar->pSpawn->ManaMax)).append(",");
	js.append("\"active\":").append(active ? "true" : "false").append(",");
	js.append("\"in_combat\":").append(combat ? "true" : "false");
	js.append("}}");
	WriteChatf("Outgoing JSON: %s", js.c_str());
	return js;
}
static void ReadSocket() {
	char buf[BUFFER_SIZE];
	int n = recv(g_socket, buf, BUFFER_SIZE - 1, 0);
	if (n > 0) {
		buf[n] = '\0';
		std::string msg(buf);
		// strip CR/LF
		msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
		msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
		std::lock_guard<std::mutex> lk(g_inMu);
		g_inQueue.push_back(msg);
	}
	else if (n <= 0) {
		safe_close_socket();
	}
}
void AnnounceMaster() {
	static bool wasActive = false;
	bool isNowActive = IsActiveClient();
	if (isNowActive && !wasActive) {
		auto* pChar = GetCharInfo();
		if (pChar && pChar->pSpawn) {
			// 1) Update the window title
			SetEQWindowTitle();

			// 2) WriteChatf locally that you switched

			WriteChatf("[%s] Is now the MASTER!!", pChar->Name);

			// 3) Let the server know
			EnqueueCommand("status active");
		}
	}
	wasActive = isNowActive;
}
void SocketWorker() {
	WriteChatf("[MQ2Elixir] ⚙️ SocketWorker starting…");
	auto lastBeat = std::chrono::steady_clock::now();

	while (g_running) {
		// 0) If not connected, try to connect/ reconnect
		if (g_socket == INVALID_SOCKET) {
			WriteChatf("[MQ2Elixir] 🔌 Attempting connect to %s:%d", ELIXIR_HOST, ELIXIR_PORT);
			SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (s != INVALID_SOCKET) {
				sockaddr_in addr{};
				addr.sin_family = AF_INET;
				addr.sin_port = htons(ELIXIR_PORT);
				inet_pton(AF_INET, ELIXIR_HOST, &addr.sin_addr);
				if (connect(s, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR) {
					g_socket = s;
					WriteChatf("[MQ2Elixir] Connected to %s:%d", ELIXIR_HOST, ELIXIR_PORT);
				}
				else {
					WriteChatf("[MQ2Elixir] connect() failed; retrying…");
					closesocket(s);
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}
			}
			else {
				WriteChatf("[MQ2Elixir] socket() failed; retrying…");
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
		}

		// 1) Wait for a new message or heartbeat timeout
		std::unique_lock<std::mutex> lk(g_outMu);
		bool woke_on_msg = g_outCv.wait_for(
			lk,
			HEARTBEAT_INTERVAL,
			[] { return !g_outQueue.empty(); }
		);
		size_t queued = g_outQueue.size();
		lk.unlock();

		WriteChatf(
			"[MQ2Elixir] ⏳ Woke up; reason = %s; queue size = %zu",
			woke_on_msg ? "new message" : "timeout",
			queued
		);

		// 2) Flush outgoing (will actually send now that g_socket is valid)
		WriteChatf("[MQ2Elixir] Calling FlushOutgoing()");
		FlushOutgoing();

		// 3) Heartbeat if needed
		auto now = std::chrono::steady_clock::now();
		if (now - lastBeat >= HEARTBEAT_INTERVAL) {
			lastBeat = now;
			WriteChatf("[MQ2Elixir] Heartbeat → enqueue character JSON");
			EnqueueCommand(BuildCharacterJson());
		}

		// 4) Read any inbound data
		WriteChatf("[MQ2Elixir] Calling ReadSocket()");
		ReadSocket();
	}

	// Cleanup on exit
	WriteChatf("[MQ2Elixir] SocketWorker exiting; closing socket");
	safe_close_socket();
}

void HandleElixirMessage(const std::string& raw) {
	auto* pChar = GetCharInfo();
	if (!pChar || !pChar->pSpawn) return;
	std::string msg = raw;
	WriteChatf("🐛 RAW_IN: %s", msg.c_str());
	if (!msg.empty() && msg.front()   == '[') {
		auto close = msg.find(']');
		if (close != std::string::npos && msg.size() > close + 2 && msg[close + 1] == ' ')
			msg = msg.substr(close + 2);
	}
	if (!msg.empty() && msg.back()    == ']') msg.pop_back();
	if (_stricmp(msg.c_str(), "ping") == 0) {
		EnqueueCommand("pong");
		return;
	}
	if (msg.rfind("cmd switch ", 0)   == 0) {
		auto tgt = msg.substr(11);
		std::string title = std::string("EverQuest - ") + tgt;
		if (HWND hwnd = FindWindowA(nullptr, title.c_str())) {
			WriteChatf("Switching to %s", tgt.c_str());
			SetForegroundWindow(hwnd);
		}
		return;
	}
	if (!msg.empty() && msg.front()   == '/') {
		DoCommandf("%s", msg.c_str());
		return;
	}
	if (msg.rfind("cmd ", 0) == 0) {
		auto sep = msg.find(' ', 4);
		if (sep != std::string::npos) {
			auto target = msg.substr(4, sep - 4);
			auto action = msg.substr(sep + 1);
			if (_stricmp(target.c_str(), pChar->Name) == 0) {
				DoCommandf("%s", action.c_str());
			}
		}
		return;
	}
	WriteChatf("Unhandled: %s", msg.c_str());
}
void TryRegisterCharacter()
{
	if (g_registered)
		return;

	auto* pChar = GetCharInfo();
	if (pChar && pChar->pSpawn && pChar->Name[0] != '\0')
	{
		// send your full JSON character payload or just the name,
		// here I’ll reuse your BuildCharacterJson for an initial heartbeat:
		EnqueueCommand(BuildCharacterJson());
		g_registered = true;
		WriteChatf("Registered with Elixir as %s", pChar->Name);
	}
}
PLUGIN_API void OnPulse() {
	TryRegisterCharacter();      // ← now compiles, pSpawn is callable
	AnnounceMaster();
	std::lock_guard<std::mutex> lk(g_inMu);
	while (!g_inQueue.empty()) {
		HandleElixirMessage(g_inQueue.front());
		g_inQueue.pop_front();
	}
}
PLUGIN_API void ElixirCommand(PSPAWNINFO, PCHAR line) {
	WriteChatf("Firing ElixirCommand(line)");
	if (line && *line) {
		// Show that we got the slash command
		WriteChatf("[MQ2Elixir] ElixirCommand fired: %s", line);
		EnqueueCommand(line);
	}
	else {
		WriteChatf("[MQ2Elixir] 🔧 Usage: /elixir cmd <target> <action>");
	}
}
PLUGIN_API void EcaCommand(PSPAWNINFO, PCHAR line) {
	auto* p = GetCharInfo(); if (!p || !p->pSpawn) { WriteChatf("Not WriteChatfged in."); return; }
	if (!line || !*line) { WriteChatf("Usage: /eca <cmd>"); return; }
	EnqueueCommand(std::string("cmd all ") + line);
}
PLUGIN_API void SwitchWindowCommand(PSPAWNINFO, PCHAR line) {
	if (line && *line) EnqueueCommand(std::string("cmd switch ") + line);
}
PLUGIN_API void OnWriteChatColor(const char* line, DWORD) {
	if (strstr(line, "invited you to join their group.")) {
		DoCommandf("/keypress ctrl+i");
	}
}
PLUGIN_API void InitializePlugin() {
	DebugSpewAlways("MQ2Elixir::InitializePlugin");
	DebugSpewAlways("MQ2Elixir::InitializePlugin called");
	WriteChatf("[MQ2Elixir] InitializePlugin");

	AddCommand("/elixir", ElixirCommand);
	AddCommand("/switch", SwitchWindowCommand);
	AddCommand("/eca", EcaCommand);
	g_running = true;
	g_socketThread = std::thread(SocketWorker);
	WriteChatf("[MQ2Elixir] 📡 SocketWorker thread started");
}
PLUGIN_API void ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin");

	// 1) signal the worker to exit its loop
	g_running = false;

	// 2) immediately close the socket to break out of any blocking I/O
	safe_close_socket();

	// 3) wake up any thread waiting on g_outCv (notify_all is safest)
	g_outCv.notify_all();

	// 4) join the socket‐worker thread exactly once
	if (g_socketThread.joinable()) {
		WriteChatf("[MQ2Elixir] Waiting for SocketWorker to exit...");
		g_socketThread.join();
		WriteChatf("[MQ2Elixir] SocketWorker thread stopped");
	}

	// 5) tear down commands
	RemoveCommand("/elixir");
	RemoveCommand("/switch");
	RemoveCommand("/eca");

	DebugSpewAlways("MQ2Elixir::ShutdownPlugin complete");
}

PLUGIN_API void OnZoned()
{
  g_registered = false;
}
