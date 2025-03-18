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

	std::string command = szArg1;
	std::string argument = szArg2;

	if (!client) {
		client = new Client();
	}

	if (client->request(command, argument)) {
		WriteChatf("MQ2Elixir: Command executed: %s %s", command.c_str(), argument.c_str());
	}
	else {
		WriteChatf("MQ2Elixir: Invalid command. Usage: /elixir connect <ip>:<port> | /elixir disconnect | /elixir send <message>");
	}
}

PLUGIN_API VOID InitializePlugin(VOID) {
	AddCommand("/elixir", ElixirCommand);
	WriteChatf("MQ2Elixir: Plugin loaded.");
}

PLUGIN_API VOID ShutdownPlugin(VOID) {
	RemoveCommand("/elixir");
	if (client) {
		client->request("disconnect", "");
		delete client;
		client = nullptr;
	}
	WriteChatf("MQ2Elixir: Plugin unloaded.");
}
