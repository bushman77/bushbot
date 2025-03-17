#include <mq/Plugin.h>
#include <windows.h>
#include <string>
#include "Client.h"

// We rely on Client.h to include <winhttp.h>, so we don't need to include it here.
#pragma comment(lib, "winhttp.lib")

PreSetup("MQ2Elixir");
PLUGIN_VERSION(0.1);

Client* client = nullptr;

/**
 * @brief Command to connect to the WebSocket server.
 *
 * Usage: /elixir connect [host] [port]
 * Defaults: host = "10.0.0.9", port = 4000.
 */
VOID ElixirConnectCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	// Default values
	char host[256] = "10.0.0.9";
	int port = 4000;

	// Parse optional host and port from the command line
	if (szLine && szLine[0])
	{
		// Validate input: require both host and port
		if (sscanf(szLine, "%255s %d", host, &port) != 2)
		{
			WriteChatf("MQ2Elixir: Invalid input. Usage: /elixir connect <host> <port>");
			return;
		}
	}

	std::string userAgent = "MQ2Elixir WebSocket Client";

	// Start the WinHTTP session
	HINTERNET hSession = client->Start_Session(userAgent);
	if (!hSession)
	{
		WriteChatf("MQ2Elixir: Failed to start session.");
		return;
	}

	// Connect to the WebSocket server and join the default channel (room:lobby)
	HINTERNET hConnect = client->Connect(hSession, host, port);
	if (hConnect)
	{
		WriteChatf("MQ2Elixir: Successfully connected to %s:%d", host, port);
	}
	else
	{
		WriteChatf("MQ2Elixir: Failed to connect to %s:%d", host, port);
		client->Cleanup(hSession, hConnect, nullptr, nullptr);
	}
}

PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Elixir::Initializing version %f", MQ2Version);
	client = new Client();
	AddCommand("/elixir connect", ElixirConnectCommand);
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Elixir::Shutting down");
	if (client)
	{
		delete client;
		client = nullptr;
	}
	RemoveCommand("/elixir connect");
}
