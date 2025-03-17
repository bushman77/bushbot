#include <mq/Plugin.h>


#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00  // Windows 10 or later
#define WINHTTP_SUPPORT_WEBSOCKETS

#include <windows.h>
#include <winhttp.h>
#include <websocket.h>

#include <string>
#include "Client.h"

#undef SendMessage  // Prevent macro conflict with Windows.h

#ifndef WINHTTP_WEB_SOCKET_UTF8_MESSAGE
#define WINHTTP_WEB_SOCKET_UTF8_MESSAGE 0x80000002
#endif

Client::Client() {
	WriteChatf("Hello, World! From Client class!");
}

void Client::Broadcast(const std::string& message) {
	WriteChatf("Client: Sending message: %s", message.c_str());
}

/*
* Here we generate our session object `session`.
* I am going to add each aspect discovered through chats with ChatGPT (yay ChatGPT!)
* one at a time and deal with each error and/or issue as they arise.
*/
//void Client::Start_Session(const std::string& character) {
HINTERNET Client::Start_Session(const std::string& userAgent) {
	// Convert the userAgent to a wide string (WinHTTP uses wide strings)
	std::wstring wUserAgent(userAgent.begin(), userAgent.end());

	// Declare the session handle and initialize to nullptr
	//HINTERNET hSession = nullptr;

	// Attempt to open the HTTP session
	HINTERNET hSession = WinHttpOpen(wUserAgent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession) {
		WriteChatf("Failed to open HTTP session: %lu", GetLastError());
		return nullptr;  // Explicitly return nullptr on failure
	}

	WriteChatf("HTTP session started successfully!");
	return hSession;  // Return the valid handle on success

	// Fallback return to satisfy the compiler (not really needed in this context)
	return nullptr;
}
HINTERNET Client::Connect(HINTERNET hSession, const std::string& host, int port) {
	std::wstring wHost(host.begin(), host.end());
	//std::wstring wPath(path.begin(), path.end());

	// Create an HTTP connection handle
	HINTERNET hConnect = WinHttpConnect(
		hSession,
		wHost.c_str(),
		port,
		0
	);

	if (!hConnect) {
		WriteChatf("Failed to connect to host: %lu", GetLastError());
		return nullptr;
	}
	WriteChatf("Connected to host!");
	std::wstring requestUrl = L"/socket/websocket?vsn=2.0.0";
	// Create a WebSocket request handle
	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect,
		L"GET",
		requestUrl.c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_SECURE
	);

	if (!hRequest) {
		WriteChatf("Failed to open WebSocket request: %lu", GetLastError());
		WinHttpCloseHandle(hConnect);
		return nullptr;
	}
	WriteChatf("WebSocket request created!");

	// Send the WebSocket handshake request
	if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0)) {
		WriteChatf("Failed to send WebSocket handshake: %lu", GetLastError());
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		return nullptr;
	}

	// Receive the WebSocket handshake response
	if (!WinHttpReceiveResponse(hRequest, nullptr)) {
		WriteChatf("Failed to receive WebSocket handshake response: %lu", GetLastError());
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		return nullptr;
	}

	WriteChatf("WebSocket handshake successful!");

	// Upgrade the HTTP connection to a WebSocket connection
	HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);

	if (!hWebSocket) {
		WriteChatf("Failed to complete WebSocket upgrade: %lu", GetLastError());
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		return nullptr;
	}

	WriteChatf("WebSocket connection established!");
	return hWebSocket;
}
bool Client::JoinChannel(HINTERNET hWebSocket, const std::string& channel) {
	std::string joinMessage = R"({"topic": "room:)" + channel + R"(","event": "phx_join","payload": {"name": "Phrogeater"},"ref": "1"})";

	// Send the join message over the WebSocket connection
	BOOL result = WinHttpWebSocketSend(
		hWebSocket,
		static_cast<_WINHTTP_WEB_SOCKET_BUFFER_TYPE>(2),  // UTF8 message buffer type,
		(void*)joinMessage.data(),
		(DWORD)joinMessage.size()
	);


	if (result == NO_ERROR) {
		WriteChatf("Successfully joined channel: %s", channel.c_str());
		return true;
	}
	else {
		WriteChatf("Failed to join channel: %s (Error code: %lu)", channel.c_str(), GetLastError());
		return false;
	}
}
void Client::Cleanup(HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest, HINTERNET hWebSocket) {
	if (hWebSocket) WinHttpCloseHandle(hWebSocket);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}

