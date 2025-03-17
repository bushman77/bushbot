#include <mq/Plugin.h>
#include <windows.h>
#include <winhttp.h>
#include <websocket.h>
#include <string>
#include "Client.h"

#pragma comment(lib, "winhttp.lib")

PreSetup("MQ2Elixir");
PLUGIN_VERSION(0.1);

//sClient* client = nullptr;
HINTERNET hSession = nullptr;
PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Elixir::Initializing version %f", MQ2Version);
	//client = new Client();
	//client->start_session();
	Client client;
	hSession = client.Start_Session("Phrogeater");
	if (hSession) {
		auto connection = client.Connect(hSession, "10.0.0.9", 4000);
		if (connection) {
			WriteChatf("Connected successfully!");
			if (client.JoinChannel(connection, "lobby")) {
				WriteChatf("Channel joined successfully!");
			}
			else {
				WriteChatf("Failed to join channel!");
			}
		}
		else {
			WriteChatf("Failed to connect to server!");
		}
	}
	else {
		WriteChatf("Failed to start session!");
	}

}


PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Elixir::Shutting down");
	/*
	if (hWebSocket) {
		WinHttpCloseHandle(hWebSocket);
		WriteChatf("MQ2Elixir: WebSocket connection closed.");
	}

	if (hConnect) {
		WinHttpCloseHandle(hConnect);
	}

	if (hSession) {
		WinHttpCloseHandle(hSession);
	}
	*/
	/*if (client)
	{
		delete client;
		client = nullptr;
	}*/
}