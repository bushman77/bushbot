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

// --- Constants ---
constexpr int BUFFER_SIZE = 4096;
constexpr const char* ELIXIR_HOST = "127.0.0.1";
constexpr int ELIXIR_PORT = 4000;

// --- Globals ---
SOCKET elixirSocket = INVALID_SOCKET;
std::thread receiverThread;
std::atomic<bool> running = false;

// --- Forward Declarations ---
bool ConnectToElixir();
void DisconnectFromElixir();
bool SendToElixir(const std::string& msg);
void StartReceiverThread();
void HandleElixirMessage(const std::string& msg);
void ElixirCommand(PSPAWNINFO, PCHAR szLine);

// --- Handle incoming Elixir messages ---
void HandleElixirMessage(const std::string& raw) {
	WriteChatf("[Elixir] %s", raw.c_str());

	if (!GetCharInfo() || !GetCharInfo()->pSpawn) return;
	std::string myName = GetCharInfo()->Name;

	// Strip out prefix like "[Name] says: "
	size_t cmdPos = raw.find("cmd ");
	if (cmdPos == std::string::npos) return;

	std::string msg = raw.substr(cmdPos); // grab from actual cmd onward
	WriteChatf("🔍 Sanitized msg: %s", msg.c_str());

	// Now parse: cmd <target> <command>
	size_t firstSpace = msg.find(' ', 4);
	if (firstSpace == std::string::npos) return;

	std::string target = msg.substr(4, firstSpace - 4);
	std::string command = msg.substr(firstSpace + 1);

	WriteChatf("[DEBUG] Matched name: %s, will run: %s", myName.c_str(), command.c_str());

	if (_stricmp(target.c_str(), myName.c_str()) == 0) {
		WriteChatf("[MQ2Elixir] Executing command for %s: %s", myName.c_str(), command.c_str());
		DoCommand(GetCharInfo()->pSpawn, command.c_str());
	}
}


// --- Background Receiver Thread ---
void StartReceiverThread() {
	if (receiverThread.joinable()) return;

	running = true;
	receiverThread = std::thread([]() {
		char buffer[BUFFER_SIZE];
		while (running && elixirSocket != INVALID_SOCKET) {
			int bytes = recv(elixirSocket, buffer, sizeof(buffer) - 1, 0);
			if (bytes > 0) {
				buffer[bytes] = '\0';
				std::string msg(buffer);
				msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
				msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());
				HandleElixirMessage(msg);
			}
			else {
				break;
			}
		}
		});
}

// --- Connect to Elixir ---
bool ConnectToElixir() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		WriteChatf("[MQ2Elixir] WSAStartup failed.");
		return false;
	}

	elixirSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (elixirSocket == INVALID_SOCKET) {
		WriteChatf("[MQ2Elixir] Failed to create socket.");
		WSACleanup();
		return false;
	}

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(ELIXIR_PORT);
	inet_pton(AF_INET, ELIXIR_HOST, &serverAddr.sin_addr);

	char addrBuf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &serverAddr.sin_addr, addrBuf, sizeof(addrBuf));
	WriteChatf("[MQ2Elixir] Connecting to %s:%d...", addrBuf, ELIXIR_PORT);

	if (connect(elixirSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		WriteChatf("[MQ2Elixir] Failed to connect: %d", WSAGetLastError());
		closesocket(elixirSocket);
		WSACleanup();
		elixirSocket = INVALID_SOCKET;
		return false;
	}

	if (GetCharInfo()) {
		std::string charName = GetCharInfo()->Name;
		if (!SendToElixir(charName)) {
			WriteChatf("[MQ2Elixir] Failed to send character name.");
			DisconnectFromElixir();
			return false;
		}
		else {
			WriteChatf("[MQ2Elixir] Sent character name: %s", charName.c_str());
		}
	}

	WriteChatf("[MQ2Elixir] Connected successfully.");
	StartReceiverThread();
	return true;
}

// --- Disconnect Cleanly ---
void DisconnectFromElixir() {
	running = false;

	if (elixirSocket != INVALID_SOCKET) {
		shutdown(elixirSocket, SD_BOTH);
		closesocket(elixirSocket);
		elixirSocket = INVALID_SOCKET;
	}

	if (receiverThread.joinable()) {
		receiverThread.join();
	}

	WSACleanup();
	WriteChatf("[MQ2Elixir] Disconnected from Elixir.");
}

// --- Send Message to Elixir ---
bool SendToElixir(const std::string& msg) {
	if (elixirSocket == INVALID_SOCKET) {
		WriteChatf("[MQ2Elixir] Socket not connected.");
		return false;
	}

	std::string data = msg + "\n";
	int sent = send(elixirSocket, data.c_str(), static_cast<int>(data.size()), 0);
	if (sent == SOCKET_ERROR) {
		WriteChatf("[MQ2Elixir] Send failed: %d", WSAGetLastError());
		DisconnectFromElixir();
		return false;
	}
	return true;
}

// --- /elixir Command Handler ---
void ElixirCommand(PSPAWNINFO, PCHAR szLine) {
	if (elixirSocket == INVALID_SOCKET && !ConnectToElixir()) {
		WriteChatf("[MQ2Elixir] Unable to connect to Elixir.");
		return;
	}

	if (!SendToElixir(szLine)) {
		WriteChatf("[MQ2Elixir] Failed to send command.");
	}
}

// --- Plugin API ---
PLUGIN_API VOID InitializePlugin() {
	DebugSpewAlways("MQ2Elixir::InitializePlugin()");
	AddCommand("/elixir", ElixirCommand);
	ConnectToElixir();
}

PLUGIN_API VOID ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin()");
	RemoveCommand("/elixir");
	DisconnectFromElixir();
}
