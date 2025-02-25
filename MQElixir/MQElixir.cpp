#include "../MQ2Plugin.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <filesystem>

PreSetup("MQ2Elixir");

std::thread elixirThread;
bool elixirRunning = false;
FILE* elixirPipe = nullptr;

// Function to start the Elixir node
void StartElixirNode()
{
	WriteChatf("Starting Elixir Node...");

	// Open a pipe to Elixir process
	const char* command = "elixir C:\\MacroQuest\\elixir\\mq_genserver.exs";
	elixirPipe = _popen(command, "w");


	if (!elixirPipe) {
		WriteChatf("Failed to start Elixir Node.");
	}
	else {
		WriteChatf("Elixir Node Started.");
	}
}

// Function to send a message to Elixir
void SendMessageToElixir(const std::string& message)
{
	if (elixirPipe) {
		fprintf(elixirPipe, "%s\n", message.c_str());
		fflush(elixirPipe);
		WriteChatf("Message as been sent");
	}
}

// Plugin initialization
PLUGIN_API void InitializePlugin()
{
	WriteChatf("MQ2Elixir Loaded - Connecting to Elixir Node...");

	elixirRunning = true;
	elixirThread = std::thread(StartElixirNode);
}

// Plugin shutdown
PLUGIN_API void ShutdownPlugin()
{
	WriteChatf("MQ2Elixir Unloaded - Stopping Elixir Node...");

	elixirRunning = false;
	if (elixirPipe) {
		_pclose(elixirPipe);
	}
	if (elixirThread.joinable()) {
		elixirThread.join();
	}
}

// Example MacroQuest command
PLUGIN_API void MQ2ElixirCommand(PSPAWNINFO pSpawn, char* szLine)
{
	WriteChatf("Sending Message to Elixir: %s", szLine);
	SendMessageToElixir(szLine);
}

// Register the command
PLUGIN_API void SetGameState(DWORD GameState)
{
	if (GameState == GAMESTATE_INGAME) {
		AddCommand("/elixir", MQ2ElixirCommand);
	}
}
