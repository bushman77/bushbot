#pragma once

#include <windows.h>
#include <winhttp.h>
#include <string>

class Client
{
public:
	Client();
	void to_server(const std::string& message); // Renamed from Broadcast
	HINTERNET Start_Session(const std::string& userAgent);
	HINTERNET Connect(HINTERNET hSession, const std::string& host, int port);
	void Cleanup(HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest, HINTERNET hWebSocket);
};
