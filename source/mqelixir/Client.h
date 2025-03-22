#pragma once
#include <string>
#include <iostream>
#include <windows.h>
#include <winhttp.h>
#include <websocket.h>
extern HINTERNET room_lobby;

class Client {

public:

	Client();
	~Client();

	void connect(const std::string& serverAddress, int port);
	void join(const std::string& channel);
	void disconnect();

private:
	HINTERNET hRequest;
	HINTERNET hSession;
	HINTERNET hConnection;
	HINTERNET hWebSocket;
};

