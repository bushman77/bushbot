// File: MQ2Elixir.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>
#include <cstdarg>
#include "../MQ2Plugin.h"
#include "MQ2Elixir.h"

#pragma comment(lib, "ws2_32.lib")

PreSetup("MQ2Elixir");

// ──────────────────────────────────────────────────────────────
// Config & globals
// ──────────────────────────────────────────────────────────────
constexpr int   BUFFER_SIZE = 4096;
constexpr char  ELIXIR_HOST[] = "127.0.0.1";
constexpr int   ELIXIR_PORT = 4000;
constexpr int   STANDSTATE_COMBAT = 100;

static SOCKET                        g_socket = INVALID_SOCKET;
static std::atomic<bool>            g_running{ false };
static std::deque<std::string>      g_outgoing;
static std::mutex                   g_outMu;
static std::condition_variable      g_outCv;
static std::thread                   g_ioThread;

// ──────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────
bool IsActiveClient() {
	return GetForegroundWindow() == GetEQWindowHandle();
}

void safe_close_socket() {
	if (g_socket != INVALID_SOCKET) {
		closesocket(g_socket);
		g_socket = INVALID_SOCKET;
	}
}

void Log(const char* fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
	va_end(args);
	WriteChatf("[MQ2Elixir] %s", buf);
}

// Thread-safe enqueue
void EnqueueCommand(const std::string& cmd) {
	{
		std::lock_guard<std::mutex> lk(g_outMu);
		g_outgoing.push_back(cmd + "\n");
	}
	g_outCv.notify_one();
}

// Build the JSON payload for this character
std::string BuildCharacterJson() {
	auto* pChar = GetCharInfo();
	auto* pChar2 = GetCharInfo2();
	if (!pChar || !pChar->pSpawn || !pChar2) return "{}";

	bool active = IsActiveClient();
	bool inCombat = pChar->pSpawn->StandState == STANDSTATE_COMBAT;

	std::string js = "{";
	js += R"("character":{)";
	js += "\"name\":\"" + std::string(pChar->Name) + "\",";
	js += "\"class_id\":" + std::to_string(pChar->pSpawn->mActorClient.Class) + ",";
	js += "\"level\":" + std::to_string(pChar->pSpawn->Level) + ",";
	js += "\"hp\":" + std::to_string(pChar->pSpawn->HPCurrent) + ",";
	js += "\"max_hp\":" + std::to_string(pChar->pSpawn->HPMax) + ",";
	js += "\"mp\":" + std::to_string(pChar->pSpawn->ManaCurrent) + ",";
	js += "\"max_mp\":" + std::to_string(pChar->pSpawn->ManaMax) + ",";
	js += "\"active\":" + std::string(active ? "true" : "false") + ",";
	js += "\"in_combat\":" + std::string(inCombat ? "true" : "false");
	js += "}}";
	return js;
}

// Attempt TCP connect, set non-blocking + keepalive
bool TryConnect() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

	g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_socket == INVALID_SOCKET) { WSACleanup(); return false; }

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ELIXIR_PORT);
	inet_pton(AF_INET, ELIXIR_HOST, &addr.sin_addr);

	if (connect(g_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		safe_close_socket();
		WSACleanup();
		return false;
	}

	// Non-blocking
	u_long mode = 1;
	ioctlsocket(g_socket, FIONBIO, &mode);
	// Keep-alive
	BOOL keep = TRUE;
	setsockopt(g_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&keep, sizeof(keep));

	Log("✅ Connected to Elixir server");
	return true;
}

// ──────────────────────────────────────────────────────────────
// I/O Thread: connects, then loops sending/receiving + heartbeat
// ──────────────────────────────────────────────────────────────
void IoThread() {
	while (g_running) {
		if (!TryConnect()) {
			Log("🔄 Retry connect in 5s…");
			std::this_thread::sleep_for(std::chrono::seconds(5));
			continue;
		}

		auto last_beat = std::chrono::steady_clock::now();

		while (g_running && g_socket != INVALID_SOCKET) {
			// 1) Flush outgoing queue
			{
				std::lock_guard<std::mutex> lk(g_outMu);
				while (!g_outgoing.empty() && g_socket != INVALID_SOCKET) {
					send(g_socket, g_outgoing.front().c_str(),
						int(g_outgoing.front().size()), 0);
					g_outgoing.pop_front();
				}
			}

			// 2) Read incoming (non-blocking)
			char buf[BUFFER_SIZE];
			int n = recv(g_socket, buf, BUFFER_SIZE - 1, 0);
			if (n > 0) {
				buf[n] = 0;
				std::string msg(buf);
				msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
				msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
				HandleElixirMessage(msg);
			}
			else if (n == 0 || (n < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
				Log("❌ Disconnected, reconnecting…");
				safe_close_socket();
				WSACleanup();
				break;
			}

			// 3) Heartbeat every second
			auto now = std::chrono::steady_clock::now();
			if (now - last_beat >= std::chrono::seconds(1)) {
				last_beat = now;
				EnqueueCommand(BuildCharacterJson());
			}

			// 4) Wait for new outgoing or 50ms timeout
			std::unique_lock<std::mutex> lk(g_outMu);
			g_outCv.wait_for(lk, std::chrono::milliseconds(50),
				[] { return !g_outgoing.empty(); });
		}
	}
}

// ──────────────────────────────────────────────────────────────
// Incoming-message handler (must match exactly)
// ──────────────────────────────────────────────────────────────
void HandleElixirMessage(const std::string& raw) {
	auto* pChar = GetCharInfo();
	if (!pChar || !pChar->pSpawn) return;
	std::string myName(pChar->Name);

	// Log raw payload
	Log("🐛 RAW_INCOMING: \"%s\"", raw.c_str());

	// Strip optional "[Sender] " prefix
	std::string msg = raw;
	if (!msg.empty() && msg.front() == '[') {
		auto close = msg.find(']');
		if (close != std::string::npos && msg.size() > close + 2 && msg[close + 1] == ' ')
			msg = msg.substr(close + 2);
	}
	// Remove stray trailing ']'
	if (!msg.empty() && msg.back() == ']')
		msg.pop_back();

	// 1) Built-in responses
	if (_stricmp(msg.c_str(), "pong") == 0) {
		WriteChatf("pong");
		return;
	}
	if (msg.rfind("cmd switch ", 0) == 0) {
		std::string tgt = msg.substr(11);
		std::string title = "EverQuest - " + tgt;
		if (HWND hwnd = FindWindowA(nullptr, title.c_str())) {
			Log("🔄 Switching to window: %s", tgt.c_str());
			SetForegroundWindow(hwnd);
		}
		return;
	}

	// 2) Forced slash commands always run
	if (!msg.empty() && msg.front() == '/') {
		Log("⚡️ [FORCED] %s → %s", myName.c_str(), msg.c_str());
		DoCommandf("%s", msg.c_str());
		return;
	}

	// 3) Dispatch any “cmd target action” for this character
	if (msg.rfind("cmd ", 0) == 0) {
		size_t sep = msg.find(' ', 4);
		if (sep != std::string::npos) {
			std::string target = msg.substr(4, sep - 4);
			std::string action = msg.substr(sep + 1);
			if (_stricmp(pChar->Name, target.c_str()) == 0) {
				Log("⚙️ Executing %s for %s", action.c_str(), target.c_str());
				DoCommandf("%s", action.c_str());
			}
		}
		return;
	}

	// 4) Anything else
	Log("⚠️ Unhandled incoming: %s", msg.c_str());
}

// ──────────────────────────────────────────────────────────────
// Plugin commands now enqueue, not send directly
// ──────────────────────────────────────────────────────────────
PLUGIN_API void ElixirCommand(PSPAWNINFO, PCHAR line) {
	if (!line || !*line) return;
	EnqueueCommand(std::string(line));
}
PLUGIN_API void EcaCommand(PSPAWNINFO, PCHAR line) {
	auto* p = GetCharInfo();
	if (!p || !p->pSpawn) { Log("❌ Not logged in."); return; }
	if (!line || !*line) { Log("Usage: /eca <cmd>"); return; }
	EnqueueCommand("cmd all " + std::string(line));
	Log("📢 ECA → cmd all %s", line);
}
PLUGIN_API void SwitchWindowCommand(PSPAWNINFO, PCHAR line) {
	if (!line || !*line) { Log("Usage: /switch <Name>"); return; }
	EnqueueCommand("cmd switch " + std::string(line));
}

// auto-accept invites remains unchanged
PLUGIN_API void OnWriteChatColor(const char* line, DWORD) {
	if (strstr(line, "invited you to join their group.")) {
		Log("🤝 Auto-accepting group invite");
		DoCommandf("/keypress ctrl+i");
	}
}

// ──────────────────────────────────────────────────────────────
// Plugin lifecycle: spin up / tear down I/O thread
// ──────────────────────────────────────────────────────────────
PLUGIN_API void InitializePlugin() {
	DebugSpewAlways("MQ2Elixir::InitializePlugin");
	AddCommand("/elixir", ElixirCommand);
	AddCommand("/switch", SwitchWindowCommand);
	AddCommand("/eca", EcaCommand);
	g_running = true;
	g_ioThread = std::thread(IoThread);
}

PLUGIN_API void ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin");
	RemoveCommand("/elixir");
	RemoveCommand("/switch");
	RemoveCommand("/eca");
	g_running = false;
	g_outCv.notify_all();
	if (g_ioThread.joinable()) g_ioThread.join();
	safe_close_socket();
	WSACleanup();
}
