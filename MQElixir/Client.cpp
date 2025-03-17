#include "Client.h"
#include <mq/Plugin.h>
#include <string>
#include <windows.h>
#undef byte
#include <websocket.h>
#include <winhttp.h>

// Constructor
Client::Client() {
	WriteChatf("MQ2Elixir: Client instance created.");
}

// Send message to the server
void Client::to_server(const std::string& message) {
	WriteChatf("MQ2Elixir: Sending message to server: %s", message.c_str());
}

// Start a WebSocket session
HINTERNET Client::Start_Session(const std::string& userAgent) {
	std::wstring wUserAgent = std::wstring(userAgent.begin(), userAgent.end());
	HINTERNET hSession = WinHttpOpen(wUserAgent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession) {
		WriteChatf("MQ2Elixir Error: Failed to start session. Error: %d", GetLastError());
	}
	return hSession;
}

// Connect to the WebSocket server and join room:lobby channel
HINTERNET Client::Connect(HINTERNET hSession, const std::string& host, int port) {
	std::wstring wHost = std::wstring(host.begin(), host.end());
	HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(), port, 0);

	if (!hConnect) {
		WriteChatf("MQ2Elixir Error: Failed to connect to server. Error: %d", GetLastError());
		return nullptr;
	}

	std::wstring wsUrl = L"/ws";
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wsUrl.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!hRequest) {
		WriteChatf("MQ2Elixir Error: Failed to open request. Error: %d", GetLastError());
		return nullptr;
	}

	BOOL sent = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0);
	if (!sent) {
		WriteChatf("MQ2Elixir Error: Failed to send request. Error: %d", GetLastError());
		WinHttpCloseHandle(hRequest);
		return nullptr;
	}

	BOOL received = WinHttpReceiveResponse(hRequest, NULL);
	if (!received) {
		WriteChatf("MQ2Elixir Error: Failed to receive response. Error: %d", GetLastError());
		WinHttpCloseHandle(hRequest);
		return nullptr;
	}

	HINTERNET hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, NULL);
	if (!hWebSocket) {
		WriteChatf("MQ2Elixir Error: Failed to complete WebSocket upgrade. Error: %d", GetLastError());
		WinHttpCloseHandle(hRequest);
		return nullptr;
	}

	// Join room:lobby channel using the static cast for the buffer type
	std::string joinMessage = "{\"topic\":\"room:lobby\",\"event\":\"phx_join\",\"payload\":{},\"ref\":null}";
	DWORD bytesWritten;
	WinHttpWebSocketSend(hWebSocket, static_cast<_WINHTTP_WEB_SOCKET_BUFFER_TYPE>(2), (void*)joinMessage.c_str(), (DWORD)joinMessage.length());
	WriteChatf("MQ2Elixir: Joined room:lobby channel.");

	return hWebSocket;
}

// Cleanup resources
void Client::Cleanup(HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest, HINTERNET hWebSocket) {
	if (hWebSocket) WinHttpCloseHandle(hWebSocket);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	WriteChatf("MQ2Elixir: Cleanup complete.");
}
