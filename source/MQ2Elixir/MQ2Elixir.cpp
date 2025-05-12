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
	{
		std::lock_guard<std::mutex> lk(g_outMu);
		g_outQueue.push_back(cmd + "\n");
	}
	g_outCv.notify_one();
}
static void FlushOutgoing() {
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

	js.reserve(256);  // optional: avoid reallocs

	js.append("{\"character\":{");
	js.append("\"name\":\"").append(pChar->Name).append("\",");
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
	static bool first = true;
	if (first) {
		WriteChatf("[MQ2Elixir] SocketWorker starting");
		first = false;
	}

	while (g_running) {
		if (g_socket == INVALID_SOCKET) {
			// Blocking connect on localhost
			g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (g_socket == INVALID_SOCKET) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(ELIXIR_PORT);
			inet_pton(AF_INET, ELIXIR_HOST, &addr.sin_addr);
			if (connect(g_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
				WriteChatf("Connect failed, retrying...");
				safe_close_socket();
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			WriteChatf("Connected to Elixir server");
		}

		// Flush any outbound
		FlushOutgoing();
		// Read any inbound
		ReadSocket();

		// Heartbeat every second
		// Heartbeat every 5 seconds
		static auto lastBeat = std::chrono::steady_clock::now();
		auto now = std::chrono::steady_clock::now();

		if (now - lastBeat >= HEARTBEAT_INTERVAL) {
			lastBeat = now;
			EnqueueCommand(BuildCharacterJson());
		}


		// Wait for either new outbound or timeout
		std::unique_lock<std::mutex> lk(g_outMu);
		g_outCv.wait_for(lk, std::chrono::milliseconds(50), [] {
			WriteChatf("Wueuing message to elixir");
			return !g_outQueue.empty();
			});
	}
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
	if (line && *line) {
		// Show that we got the slash command
		WriteChatf("[MQ2Elixir] 🔧 ElixirCommand fired: %s", line);
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
}
PLUGIN_API void ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin");

	// Stop the worker loop
	g_running = false;

	// Break any blocking I/O so the thread can wake up
	safe_close_socket();

	// Wake up the wait_for in FlushOutgoing if it’s still pending
	g_outCv.notify_all();

	// Now join the socket thread promptly without freezing EQ
	if (g_socketThread.joinable()) {
		g_socketThread.join();
	}

	// Finally, remove our commands
	RemoveCommand("/elixir");
	RemoveCommand("/switch");
	RemoveCommand("/eca");
}
PLUGIN_API void OnZoned()
{
  g_registered = false;
}
