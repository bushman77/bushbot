#include <mq/Plugin.h>
#include <windows.h>
#include <string>
#include "Client.h"

// We rely on Client.h to include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

PreSetup("MQ2Elixir");
PLUGIN_VERSION(0.1);

Client* client = nullptr;

/**
 * @brief Command to attempt connection to the WebSocket server.
 *
 * Usage: /elixir connect [host] [port]
 * Defaults: host = "10.0.0.9", port = 4000.
 */
VOID ElixirConnectCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	// Default values
	char host[256] = "10.0.0.9";
	int port = 4000;

	// Parse optional host and port from the command line.
	if (szLine && szLine[0])
	{
		if (sscanf(szLine, "%255s %d", host, &port) != 2)
		{
			WriteChatf("MQ2Elixir: Invalid input. Usage: /elixir connect <host> <port>");
			return;
		}
	}

	std::string userAgent = "MQ2Elixir WebSocket Client";
	HINTERNET hSession = client->Start_Session(userAgent);
	if (!hSession)
	{
		WriteChatf("MQ2Elixir: Failed to start session. Connection could not be established.");
		return;
	}

	HINTERNET hConnect = client->Connect(hSession, host, port);
	if (hConnect)
	{
		WriteChatf("MQ2Elixir: Successfully connected to %s:%d", host, port);
	}
	else
	{
		WriteChatf("MQ2Elixir: Failed to connect to %s:%d. Use /elixir connect to retry.", host, port);
		client->Cleanup(hSession, hConnect, nullptr, nullptr);
	}
}

/**
 * @brief Command to send a message to the server.
 *
 * Usage: /esend <message>
 */
VOID ElixirSendCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (!szLine || !szLine[0])
	{
		WriteChatf("MQ2Elixir: Usage: /esend <message>");
		return;
	}
	client->to_server(szLine);
}

PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Elixir::Initializing version %f", MQ2Version);
	client = new Client();

	// Attempt automatic connection during initialization.
	std::string userAgent = "MQ2Elixir WebSocket Client";
	HINTERNET hSession = client->Start_Session(userAgent);
	if (!hSession)
	{
		WriteChatf("MQ2Elixir: Automatic connection failed to start session. Use /elixir connect to retry.");
	}
	else
	{
		const char* host = "10.0.0.9";
		int port = 4000;
		HINTERNET hConnect = client->Connect(hSession, host, port);
		if (hConnect)
		{
			WriteChatf("MQ2Elixir: Automatically connected to %s:%d", host, port);
		}
		else
		{
			WriteChatf("MQ2Elixir: Automatic connection failed to connect to %s:%d. Use /elixir connect to retry.", host, port);
			client->Cleanup(hSession, hConnect, nullptr, nullptr);
		}
	}

	// Register commands so the user can manually connect or send messages.
	AddCommand("/elixir connect", ElixirConnectCommand);
	AddCommand("/esend", ElixirSendCommand);
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
	RemoveCommand("/esend");
}
