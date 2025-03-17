#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <windows.h>
#include <winhttp.h>

class Client {
public:
	Client();
	void Broadcast(const std::string& message);
	HINTERNET Start_Session(const std::string& userAgent);
	HINTERNET Connect(HINTERNET hSession, const std::string& host, int port);
	bool JoinChannel(HINTERNET hWebSocket, const std::string& channel);
	void Cleanup(HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest, HINTERNET hWebSocket);
};

#endif  // CLIENT_H
