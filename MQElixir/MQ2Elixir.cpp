#include <mq/Plugin.h>
#include <windows.h>
#include <string>
#include "Client.h"

#pragma comment(lib, "winhttp.lib")

PreSetup("MQ2Elixir");
PLUGIN_VERSION(0.1);

Client* client = nullptr;

// Command to connect to the WebSocket server
VOID ElixirConnectCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	char host[256] = "10.0.0.9";
	int port = 4000;

	if (szLine && szLine[0])
	{
		sscanf(szLine, "%255s %d", host, &port);
	}

	std::string userAgent = "MQ2Elixir WebSocket Client";
	std::wstring wHost = std::wstring(host, host + strlen(host));

	HINTERNET hSession = client->Start_Session(userAgent);
	if (!hSession)
	{
		WriteChatf("MQ2Elixir: Failed to start session.");
		return;
	}

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
