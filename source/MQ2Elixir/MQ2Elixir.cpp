// MQ2Elixir.cpp  (e.g. MQ2Elixir/MQ2Elixir.cpp)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <atomic>
#include "../MQ2Plugin.h"

#pragma comment(lib, "ws2_32.lib")

PreSetup("MQ2Elixir");

// ──────────────────────────────────────────────────────────────
// Config & globals
// ──────────────────────────────────────────────────────────────
constexpr int  BUFFER_SIZE = 4096;
constexpr char ELIXIR_HOST[] = "127.0.0.1";
constexpr int  ELIXIR_PORT = 4000;
constexpr int  STANDSTATE_COMBAT = 100;

SOCKET        g_socket = INVALID_SOCKET;
std::thread   g_receiver;
std::atomic<bool> g_running{ false };

// ──────────────────────────────────────────────────────────────
// Utility
// ──────────────────────────────────────────────────────────────
bool IsActiveClient() { return GetForegroundWindow() == GetEQWindowHandle(); }
void safe_close_socket()
{
	if (g_socket != INVALID_SOCKET)
	{
		closesocket(g_socket);
		g_socket = INVALID_SOCKET;
	}
}
void HandleElixirMessage(std::string raw);
bool SendToElixir(const std::string& msg);

void Log(const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
	va_end(args);
	WriteChatf("[MQ2Elixir] %s", buf);
}

void SetEQWindowTitle()
{
	if (HWND hwnd = GetEQWindowHandle(); hwnd && GetCharInfo())
	{
		std::string title = "EverQuest - " + std::string(GetCharInfo()->Name);
		SetWindowTextA(hwnd, title.c_str());
		Log("🪪 Set EQ window title: %s", title.c_str());
	}
}

// ----------------------------------------------------------------------------
// /eca <action...> → sends “cmd all <action...>” to Elixir
// ----------------------------------------------------------------------------
PLUGIN_API void EcaCommand(PSPAWNINFO, PCHAR line)
{
	auto* p = GetCharInfo();
	if (!p || !p->pSpawn)
	{
		Log("❌ You must be logged in to use /eca.");
		return;
	}

	if (!line || !*line)
	{
		Log("Usage: /eca <command>");
		return;
	}

	std::string action(line);
	std::string cmd = "cmd all " + action;
	if (SendToElixir(cmd))
	{
		Log("📢 ECA: sending \"%s\"", cmd.c_str());
	}
}

// ──────────────────────────────────────────────────────────────
// JSON registration payload
// ──────────────────────────────────────────────────────────────
std::string BuildCharacterJson()
{
	auto* pChar = GetCharInfo();
	auto* pChar2 = GetCharInfo2();
	if (!pChar || !pChar->pSpawn || !pChar2) return "{}";

	bool active = IsActiveClient();
	bool inCombat = pChar->pSpawn->StandState == STANDSTATE_COMBAT;

	std::string json = "{";
	json += R"("character":{)";
	json += "\"name\":\"" + std::string(pChar->Name) + "\",";
	json += "\"class_id\":" + std::to_string(pChar->pSpawn->mActorClient.Class) + ",";
	json += "\"level\":" + std::to_string(pChar->pSpawn->Level) + ",";
	json += "\"hp\":" + std::to_string(pChar->pSpawn->HPCurrent) + ",";
	json += "\"max_hp\":" + std::to_string(pChar->pSpawn->HPMax) + ",";
	json += "\"mp\":" + std::to_string(pChar->pSpawn->ManaCurrent) + ",";
	json += "\"max_mp\":" + std::to_string(pChar->pSpawn->ManaMax) + ",";
	json += "\"active\":" + std::string(active ? "true" : "false") + ",";
	json += "\"in_combat\":" + std::string(inCombat ? "true" : "false");
	json += "}}";
	return json;
}

// ──────────────────────────────────────────────────────────────
// Socket helpers (unchanged)
// ──────────────────────────────────────────────────────────────
bool SendToElixir(const std::string& msg)
{
	if (msg.empty() || msg == "{}") return false;
	if (g_socket == INVALID_SOCKET)
	{
		Log("❌ Socket not connected.");
		return false;
	}
	std::string payload = msg + '\n';
	if (send(g_socket, payload.c_str(), static_cast<int>(payload.size()), 0) == SOCKET_ERROR)
	{
		Log("❌ Send failed: %d", WSAGetLastError());
		safe_close_socket();
		return false;
	}
	return true;
}

void DisconnectFromElixir()
{
	g_running = false;
	shutdown(g_socket, SD_BOTH);
	safe_close_socket();
	if (g_receiver.joinable()) g_receiver.join();
	WSACleanup();
	Log("🔌 Disconnected from Elixir.");
}

void StartReceiverThread()
{
	if (g_receiver.joinable()) return;
	g_running = true;
	g_receiver = std::thread([] {
		char buffer[BUFFER_SIZE];
		while (g_running && g_socket != INVALID_SOCKET)
		{
			int bytes = recv(g_socket, buffer, BUFFER_SIZE - 1, 0);
			if (bytes > 0)
			{
				buffer[bytes] = '\0';
				std::string msg(buffer);
				msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
				msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
				HandleElixirMessage(msg);
			}
			else
			{
				Log("❌ Socket closed or error during receive.");
				break;
			}
		}
		});
}

bool ConnectToElixir()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		Log("❌ WSAStartup failed.");
		return false;
	}
	g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_socket == INVALID_SOCKET)
	{
		Log("❌ Socket creation failed.");
		WSACleanup();
		return false;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(ELIXIR_PORT);
	inet_pton(AF_INET, ELIXIR_HOST, &addr.sin_addr);

	if (connect(g_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		Log("❌ Connection failed: %d", WSAGetLastError());
		safe_close_socket();
		WSACleanup();
		return false;
	}

	std::string hello = BuildCharacterJson() + '\n';
	Log("📤 Sending JSON: %s", hello.c_str());
	if (send(g_socket, hello.c_str(), static_cast<int>(hello.size()), 0) == SOCKET_ERROR)
	{
		Log("❌ Send failed: %d", WSAGetLastError());
		DisconnectFromElixir();
		return false;
	}

	StartReceiverThread();
	Log("✅ Connected successfully.");
	return true;
}

// ──────────────────────────────────────────────────────────────
// Incoming messages (unchanged)
// ──────────────────────────────────────────────────────────────
void HandleElixirMessage(std::string raw)
{
	auto* pChar = GetCharInfo();
	if (!pChar || !pChar->pSpawn) return;
	std::string myName = pChar->Name;

	Log("🐛 IsActiveClient? %s", IsActiveClient() ? "YES" : "NO");
	Log("🐛 RAW_INCOMING: \"%s\"", raw.c_str());

	if (!raw.empty() && raw.front() == '[')
	{
		auto closing = raw.find(']');
		if (closing != std::string::npos && raw.size() > closing + 2 && raw[closing + 1] == ' ')
			raw = raw.substr(closing + 2);
	}
	if (!raw.empty() && raw.back() == ']')
		raw.pop_back();

	if (_stricmp(raw.c_str(), "pong") == 0)
	{
		WriteChatf("pong");
		return;
	}
	if (raw.rfind("cmd switch ", 0) == 0)
	{
		std::string target = raw.substr(11);
		std::string title = "EverQuest - " + target;
		if (HWND hwnd = FindWindowA(nullptr, title.c_str()))
		{
			Log("🔄 Switching to window: %s", target.c_str());
			SetForegroundWindow(hwnd);
		}
		return;
	}
	if (!raw.empty() && raw.front() == '/')
	{
		Log("⚡️ [FORCED] %s → %s", myName.c_str(), raw.c_str());
		DoCommandf("%s", raw.c_str());
		return;
	}
	if (IsActiveClient()) return;

	if (raw.rfind("cmd ", 0) == 0)
	{
		size_t sep = raw.find(' ', 4);
		if (sep != std::string::npos)
		{
			std::string target = raw.substr(4, sep - 4);
			std::string action = raw.substr(sep + 1);
			Log("🐛 Parsed target/action → '%s' / '%s'",
				target.c_str(), action.c_str());
			if (_stricmp(pChar->Name, target.c_str()) == 0)
			{
				Log("⚙️ Executing %s for %s",
					action.c_str(), target.c_str());
				DoCommandf("%s", action.c_str());
			}
		}
		return;
	}
	Log("⚠️ Unhandled incoming message: %s", raw.c_str());
}

// ──────────────────────────────────────────────────────────────
// Commands registered with MacroQuest
// ──────────────────────────────────────────────────────────────
PLUGIN_API void ElixirCommand(PSPAWNINFO, PCHAR line)
{
	if (g_socket == INVALID_SOCKET && !ConnectToElixir()) return;
	SendToElixir(line);
}

PLUGIN_API void SwitchWindowCommand(PSPAWNINFO, PCHAR line)
{
	if (!line || !*line)
	{
		Log("Usage: /switch <CharacterName>");
		return;
	}
	std::string target(line);
	std::string title = "EverQuest - " + target;
	if (HWND hwnd = FindWindowA(nullptr, title.c_str()))
	{
		Log("🔄 Switching to window: %s", target.c_str());
		SetForegroundWindow(hwnd);
		SendToElixir("cmd switch " + target);
	}
	else
	{
		Log("❌ Could not find EQ window: %s", title.c_str());
	}
}
// ──────────────────────────────────────────────────────────────
// Auto-accept group invites
// ──────────────────────────────────────────────────────────────
PLUGIN_API void OnWriteChatColor(const char* line, DWORD color)
{
	// EQ prints “<sender> has invited you to join their group.”
	// when someone /invite’s you.  We just look for that text.
	if (strstr(line, "invited you to join their group."))
	{
		Log("🤝 Auto-accepting group invite");
		// EverQuest’s own /keypress command
		DoCommandf("/keypress ctrl+i");
	}
}

// `/eca` is now registered instead of `/allfollow`
PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Elixir::InitializePlugin");
	AddCommand("/elixir", ElixirCommand);
	AddCommand("/switch", SwitchWindowCommand);
	AddCommand("/eca", EcaCommand);    // ← generic “cmd all <action>”
	SetEQWindowTitle();
	ConnectToElixir();
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin");
	RemoveCommand("/elixir");
	RemoveCommand("/switch");
	RemoveCommand("/eca");
	DisconnectFromElixir();
}

// Pulse & Zone hooks remain the same:
PLUGIN_API void AnnounceMaster()
{
	static bool wasActive = false;
	bool nowActive = IsActiveClient();

	if (nowActive && !wasActive)
	{
		if (auto* p = GetCharInfo(); p && p->pSpawn)
		{
			Log("⭐️ [%s] is now the MASTER", p->Name);
			SendToElixir("status active");
		}
	}
	else if (!nowActive && wasActive)
	{
		if (auto* p = GetCharInfo(); p && p->pSpawn)
		{
			Log("🔕 [%s] no longer active", p->Name);
			SendToElixir("status inactive");
		}
	}
	wasActive = nowActive;
}

PLUGIN_API void OnPulse() { AnnounceMaster(); }
PLUGIN_API void OnZoned() { SetEQWindowTitle(); }
