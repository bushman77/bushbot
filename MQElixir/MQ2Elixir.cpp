// MQ2Elixir.cpp

#include "MQ2Plugin.h"
#include "Client.h"

PreSetup("MQ2Elixir");

Client* client = nullptr;

void ElixirCommand(PSPAWNINFO pChar, PCHAR szLine) {
	char szArg1[MAX_STRING] = { 0 };
	char szArg2[MAX_STRING] = { 0 };

	GetArg(szArg1, szLine, 1);
	GetArg(szArg2, szLine, 2);

	if (_stricmp(szArg1, "connect") == 0) {
		if (client) {
			WriteChatf("MQ2Elixir: Already connected.");
			return;
		}
		std::string host = szArg2;
		int port = 4000;
		client = new Client();
		if (client->connect(host, port)) {
			WriteChatf("MQ2Elixir: Connected to %s:%d", host.c_str(), port);
		}
		else {
			WriteChatf("MQ2Elixir: Failed to connect to %s:%d", host.c_str(), port);
			delete client;
			client = nullptr;
		}
	}
	else if (_stricmp(szArg1, "disconnect") == 0) {
		if (client) {
			client->disconnect();
			delete client;
			client = nullptr;
			WriteChatf("MQ2Elixir: Disconnected.");
		}
		else {
			WriteChatf("MQ2Elixir: Not connected.");
		}
	}
	else {
		WriteChatf("MQ2Elixir: Invalid command. Usage: /elixir connect <host> <port> | /elixir disconnect");
	}
}

PLUGIN_API VOID InitializePlugin(VOID) {
	AddCommand("/elixir", ElixirCommand);
	WriteChatf("MQ2Elixir: Plugin loaded.");
}

PLUGIN_API VOID ShutdownPlugin(VOID) {
	RemoveCommand("/elixir");
	if (client) {
		client->disconnect();
		delete client;
		client = nullptr;
	}
	WriteChatf("MQ2Elixir: Plugin unloaded.");
}
