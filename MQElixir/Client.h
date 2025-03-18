// Client.h

#pragma once
#include <string>
#include <windows.h>
#include <winhttp.h>

class Client {
public:
	Client();
	bool connect(const std::string& host, int port);
	void disconnect();

private:
	void* hSession;
	void* hConnection;
	void* hRequest;
};
