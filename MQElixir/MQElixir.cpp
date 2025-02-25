#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "../MQ2Plugin.h"
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

PreSetup("MQ2Elixir");

std::string characterName;
bool elixirRunning = false;
void SendMessageToElixir(const std::string& message);

void MQ2ElixirCommand(PSPAWNINFO pSpawn, char* szLine) {
	std::stringstream ss(szLine);
	std::string command, argument;
	ss >> command; // First word is the command
	std::getline(ss, argument); // The rest is the argument
	argument = argument.substr(argument.find_first_not_of(" ")); // Trim leading spaces

	if (command.empty()) {
		WriteChatf("[MQ2Elixir] Usage: /elixir <command> <argument>");
		return;
	}

	if (command == "register") {
		if (argument.empty()) {
			WriteChatf("[MQ2Elixir] Usage: /elixir register <character_name>");
			return;
		}
		SendMessageToElixir("REGISTER " + argument + "\n");
	}
	else if (command == "state") {
		if (argument.empty()) {
			WriteChatf("[MQ2Elixir] Usage: /elixir state <character_name>");
			return;
		}
		SendMessageToElixir("STATE " + argument + "\n");
	}
	else {
		WriteChatf("[MQ2Elixir] Unknown command: %s", command.c_str());
	}
}

// Function to check if the GenServer is running
bool IsElixirRunning() {
	WSADATA wsaData;
	SOCKET sock;
	struct sockaddr_in server;
	bool isRunning = false;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return false;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(4000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) {
		isRunning = true;
	}

	closesocket(sock);
	WSACleanup();
	return isRunning;
}

// Function to start the Elixir process
void StartElixirNode() {
	WriteChatf("Starting Elixir Node...");
	int checkElixir = system("where elixir >nul 2>nul");
	if (checkElixir != 0) {
		WriteChatf("\ar[Error]: Elixir is not installed or not in the system PATH.");
		return;
	}

#ifdef _WIN32
	const char* command = "start /B elixir C:\\MacroQuest\\elixir\\mq_genserver.exs";
#else
	const char* command = "elixir ~/MacroQuest/elixir/mq_genserver.exs &"; // Adjust for Linux
#endif

	int result = system(command);
	if (result != 0) {
		WriteChatf("\ar[Error]: Failed to start Elixir process.");
		return;
	}
	WriteChatf("\agElixir Node Started.");
}


// Function to send messages to the GenServer
void SendMessageToElixir(const std::string& message) {
	WSADATA wsaData;
	SOCKET sock = INVALID_SOCKET;
	struct sockaddr_in server;
	bool connected = false;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		WriteChatf("\ar[Error]: WSAStartup failed.");
		return;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		WriteChatf("\ar[Error]: Socket creation failed.");
		goto cleanup;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(4000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	if (connect(sock, (struct sockaddr*)&server, sizeof(server)) == 0) {
		connected = true;
		std::string formattedMessage = message + "\n";  // Ensure newline termination
		send(sock, formattedMessage.c_str(), (int)formattedMessage.length(), 0);
		WriteChatf("\ag[MQ2Elixir]: Sent message: %s", message.c_str());
	}
	else {
		WriteChatf("\ar[Error]: Failed to send message to Elixir.");
	}

cleanup:
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
	}
	WSACleanup();
}


PLUGIN_API void InitializePlugin() {
	WriteChatf("\agMQ2Elixir Loaded - Checking Elixir Server...");
	AddCommand("/elixir", MQ2ElixirCommand);

	if (!IsElixirRunning()) {
		WriteChatf("\arElixir server is not running! Attempting to start it...");
		StartElixirNode();
		Sleep(2000); // Wait 2 seconds to allow the server to start
		if (IsElixirRunning()) {
			WriteChatf("\agElixir server started successfully!");
		}
		else {
			WriteChatf("\arFailed to start Elixir server.");
		}
	}
	else {
		WriteChatf("\agElixir server is running!");
	}
}

PLUGIN_API void SetGameState(DWORD GameState) {
	if (GameState == GAMESTATE_INGAME) {
		characterName = GetCharInfo()->Name;

		// Register character
		std::string registerMessage = "REGISTER " + characterName + "\n";
		SendMessageToElixir(registerMessage);

		// Request character state
		std::string stateMessage = "STATE " + characterName + "\n";
		SendMessageToElixir(stateMessage);
	}
}

// Improved /elixir command handler
