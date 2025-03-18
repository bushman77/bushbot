// Client.cpp

#include "Client.h"
#include <string>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

Client::Client() : hSession(nullptr), hConnection(nullptr), hRequest(nullptr) {}

bool Client::connect(const std::string& host, int port) {
	hSession = WinHttpOpen(L"MQ2Elixir/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0);
	if (!hSession) {
		return false;
	}

	std::wstring whost(host.begin(), host.end());
	hConnection = WinHttpConnect(hSession, whost.c_str(), port, 0);
	if (!hConnection) {
		WinHttpCloseHandle(hSession);
		return false;
	}

	hRequest = WinHttpOpenRequest(hConnection, L"GET", L"/socket/websocket", nullptr, nullptr, nullptr, WINHTTP_FLAG_SECURE);
	if (!hRequest) {
		WinHttpCloseHandle(hConnection);
		WinHttpCloseHandle(hSession);
		return false;
	}

	BOOL result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0);
	if (!result) {
		disconnect();
		return false;
	}

	return true;
}

void Client::disconnect() {
	if (hRequest) {
		WinHttpCloseHandle(hRequest);
		hRequest = nullptr;
	}
	if (hConnection) {
		WinHttpCloseHandle(hConnection);
		hConnection = nullptr;
	}
	if (hSession) {
		WinHttpCloseHandle(hSession);
		hSession = nullptr;
	}
}
