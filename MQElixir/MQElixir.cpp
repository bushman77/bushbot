#define WIN32_LEAN_AND_MEAN  // Prevents `windows.h` from including unnecessary headers
#define _WINSOCKAPI_         // Prevents `windows.h` from including `winsock.h`
#include <winsock2.h>  // MUST be before windows.h
#include <ws2tcpip.h>
#include <windows.h>    // Some MQ2 dependencies use this
#include "../MQ2Plugin.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <filesystem>
#include <mutex>

#pragma comment(lib, "ws2_32.lib") // Ensure Winsock library is linked

PreSetup("MQ2Elixir");

std::thread elixirThread;
bool elixirRunning = false;
FILE* elixirPipe = nullptr;
std::mutex elixirMutex;

// Function to start the Elixir process
// 
////////////////////////////////////////////////////////
///////////////start elixir node
////////////////////////////////////////////////////////
void StartElixirNode()
{
	WriteChatf("Starting Elixir Node...");

	// Check if Elixir is installed
	int checkElixir = system("where elixir >nul 2>nul");
	if (checkElixir != 0) {
		WriteChatf("\ar[Error]: Elixir is not installed or not in the system PATH.");
		return;
	}

	// Start Elixir
	const char* command = "start /B elixir C:\\MacroQuest\\elixir\\mq_genserver.exs";
	int result = system(command);

	if (result != 0)
	{
		WriteChatf("\ar[Error]: Failed to start Elixir process.");
		return;
	}

	WriteChatf("\agElixir Node Started.");
}





////////////////////////////////////////////////////////
///////////////Plugin initialization
////////////////////////////////////////////////////////
PLUGIN_API void InitializePlugin()
{
	WriteChatf("\agMQ2Elixir Loaded - Connecting to Elixir Node...");

	elixirRunning = true;

	// Run Elixir process in a separate thread and detach it
	elixirThread = std::thread(StartElixirNode);
	elixirThread.detach();
}


///////////////////////////////////////////////////////
///////////////Plugin shutdown
////////////////////////////////////////////////////////
PLUGIN_API void ShutdownPlugin()
{
	WriteChatf("\arMQ2Elixir Unloaded - Stopping Elixir Node...");

	// Attempt to shut down Elixir gracefully
	system("taskkill /IM elixir.exe /F >nul 2>nul");

	elixirRunning = false;

	if (elixirThread.joinable())
	{
		elixirThread.join();
	}

	WriteChatf("\agElixir Node has been stopped.");
}





// Function to send a message to Elixir
void SendMessageToElixir(const std::string& message)
{
	WSADATA wsaData;
	SOCKET sock = INVALID_SOCKET;
	struct sockaddr_in server;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		WriteChatf("\ar[Error]: WSAStartup failed.");
		return;
	}

	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		WriteChatf("\ar[Error]: Socket creation failed.");
		WSACleanup();
		return;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(4000);
	inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

	// Try to connect (retry up to 5 times with delays)
	int retries = 5;
	while (connect(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR && retries > 0)
	{
		WriteChatf("\ar[Error]: Failed to connect to Elixir. Retrying...");
		Sleep(1000); // Wait 1 second before retrying
		retries--;
	}

	if (retries == 0)
	{
		WriteChatf("\ar[Error]: Could not connect to Elixir after multiple attempts.");
		closesocket(sock);
		WSACleanup();
		return;
	}

	// Send message
	send(sock, message.c_str(), (int)message.length(), 0);
	closesocket(sock);
	WSACleanup();

	WriteChatf("\ag[MQ2Elixir]: Sent message: %s", message.c_str());
}




// MacroQuest command to send messages to Elixir
PLUGIN_API void MQ2ElixirCommand(PSPAWNINFO pSpawn, char* szLine)
{
	SendMessageToElixir(szLine);
}

// Register the command
PLUGIN_API void SetGameState(DWORD GameState)
{
	if (GameState == GAMESTATE_INGAME)
	{
		AddCommand("/elixir", MQ2ElixirCommand);
	}
}
