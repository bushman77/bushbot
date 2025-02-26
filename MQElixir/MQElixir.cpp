#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "../MQ2Plugin.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <queue>

#pragma comment(lib, "ws2_32.lib")

PreSetup("MQ2Elixir");

// Global Variables
std::string characterName;
bool elixirRunning = false;
bool listenerRunning = false;
SOCKET elixirSocket = INVALID_SOCKET;
std::thread listenerThread;
std::mutex queueMutex;
std::queue<std::string> messageQueue;

// Function Declarations
void SendMessageToElixir(const std::string& message);
void StartElixirListener();
void StopElixirListener();
void MessageHandler(const std::string& message);
bool IsElixirServerRunning();
void StartElixirServer();
bool ConnectToElixir();

// Improved /elixir command handler
void MQ2ElixirCommand(PSPAWNINFO pSpawn, char* szLine) {

	std::stringstream ss(szLine);
	std::string command, argument;
	ss >> command;
	std::getline(ss, argument);
	if (!argument.empty()) {
		argument = argument.substr(argument.find_first_not_of(" "));
	}

	if (command.empty()) {
		WriteChatf("[MQ2Elixir] Usage: /elixir <command> <argument>");
		return;
	}

	if (command == "connect") {
		if (IsElixirServerRunning()) {
			WriteChatf("\ag[MQ2Elixir]: Elixir server is already running.");
		}
		else {
			WriteChatf("\ay[MQ2Elixir]: No running instance detected. Starting one...");
			StartElixirServer();
			Sleep(2000); // Give the server time to start
		}

		if (ConnectToElixir()) {
			WriteChatf("\ag[MQ2Elixir]: Successfully connected to Elixir.");
		}
		else {
			WriteChatf("\ar[MQ2Elixir]: Connection failed.");
		}
	}
	else if (command == "register") {
		if (argument.empty()) {
			WriteChatf("[MQ2Elixir] Usage: /elixir register <character_name>");
			return;
		}
		WriteChatf("\ag[MQ2Elixir]: Sending registration: REGISTER %s", characterName.c_str());
		SendMessageToElixir("REGISTER " + argument);
	}
	else if (command == "state") {
		if (argument.empty()) {
			WriteChatf("[MQ2Elixir] Usage: /elixir state <character_name>");
			return;
		}
		SendMessageToElixir("STATE " + argument);
	}
	else if (command == "disconnect") {
		SendMessageToElixir("DISCONNECT");
		WriteChatf("\ag[MQ2Elixir]: Disconnected from Elixir server.");
	}
	else {
		WriteChatf("[MQ2Elixir] Unknown command: %s", command.c_str());
	}

}

// Function to establish a persistent connection to Elixir
bool ConnectToElixir() {
	WSADATA wsaData;
	struct sockaddr_in server;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		WriteChatf("\ar[Error]: WSAStartup failed.");
		return false;
	}

	elixirSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (elixirSocket == INVALID_SOCKET) {
		WriteChatf("\ar[Error]: Socket creation failed.");
		WSACleanup();
		return false;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(4000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	if (connect(elixirSocket, (struct sockaddr*)&server, sizeof(server)) != 0) {
		WriteChatf("\ar[Error]: Failed to connect to Elixir server.");
		closesocket(elixirSocket);
		WSACleanup();
		return false;
	}

	WriteChatf("\ag[MQ2Elixir] Connected to Elixir Server.");
	StartElixirListener();
	return true;
}

void StartElixirServer() {
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	std::string elixirCmd = "cmd.exe /C start elixir path\\to\\mq_genserver.exs";

	if (!CreateProcessA(NULL, (LPSTR)elixirCmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		WriteChatf("\ar[MQ2Elixir]: Failed to start Elixir server.");
	}
	else {
		WriteChatf("\ag[MQ2Elixir]: Elixir server started.");
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

bool IsElixirServerRunning() {
	SOCKET testSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (testSocket == INVALID_SOCKET) return false;

	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(4000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	bool isRunning = connect(testSocket, (struct sockaddr*)&server, sizeof(server)) == 0;
	closesocket(testSocket);

	return isRunning;
}

// Function to send messages to Elixir
void SendMessageToElixir(const std::string& message) {
	if (elixirSocket == INVALID_SOCKET) {
		WriteChatf("\ar[Error]: No active connection to Elixir server.");
		return;
	}

	std::string formattedMessage = message + "\n";
	send(elixirSocket, formattedMessage.c_str(), (int)formattedMessage.length(), 0);
}

// Listener thread function to handle incoming messages
void ListenerThreadFunc() {
	listenerRunning = true;
	char buffer[1024];

	while (listenerRunning) {
		memset(buffer, 0, sizeof(buffer));
		int bytesReceived = recv(elixirSocket, buffer, sizeof(buffer) - 1, 0);

		if (bytesReceived > 0) {
			std::string message(buffer, bytesReceived);

			// Store message in queue
			std::lock_guard<std::mutex> lock(queueMutex);
			messageQueue.push(message);
		}
		else if (bytesReceived == 0) {
			WriteChatf("\ar[MQ2Elixir]: Connection closed by server.");
			break;
		}
		else {
			WriteChatf("\ar[MQ2Elixir]: Error receiving data.");
			break;
		}
	}

	listenerRunning = false;
	closesocket(elixirSocket);
	elixirSocket = INVALID_SOCKET;
}

// Function to start the listener thread
void StartElixirListener() {
	if (listenerRunning) return;
	listenerThread = std::thread(ListenerThreadFunc);
	listenerThread.detach();
}

// Function to stop the listener thread
void StopElixirListener() {
	listenerRunning = false;
	if (elixirSocket != INVALID_SOCKET) {
		closesocket(elixirSocket);
		elixirSocket = INVALID_SOCKET;
	}
}

// Function to process messages from the queue
void ProcessMessages() {
	std::lock_guard<std::mutex> lock(queueMutex);
	while (!messageQueue.empty()) {
		std::string message = messageQueue.front();
		messageQueue.pop();
		MessageHandler(message);
	}
}

// Function to handle messages from Elixir
void MessageHandler(const std::string& message) {
	WriteChatf("\ag[MQ2Elixir] Received: %s", message.c_str());

	// Example: Handle different responses
	if (message.find("ACK REGISTER") != std::string::npos) {
		WriteChatf("\ag[MQ2Elixir]: Character registration successful.");
	}
	else if (message.find("STATE UPDATE") != std::string::npos) {
		WriteChatf("\ag[MQ2Elixir]: Received character state update.");
	}
}

// Plugin initialization
PLUGIN_API void InitializePlugin() {
	WriteChatf("\agMQ2Elixir Loaded - Checking Elixir Server...");
	AddCommand("/elixir", MQ2ElixirCommand);

	if (!ConnectToElixir()) {
		WriteChatf("\ar[MQ2Elixir]: Failed to connect to Elixir.");
	}
}

// Plugin shutdown
PLUGIN_API void ShutdownPlugin() {
	StopElixirListener();
	RemoveCommand("/elixir");
	WriteChatf("\arMQ2Elixir Unloaded.");
}

// Function to handle game state changes
PLUGIN_API void SetGameState(DWORD GameState) {
	if (GameState == GAMESTATE_INGAME) {
		characterName = GetCharInfo()->Name;

		// Register character
		SendMessageToElixir("REGISTER " + characterName);

		// Request character state
		SendMessageToElixir("STATE " + characterName);
	}

	// Process any messages from Elixir
	ProcessMessages();
}
