#include "Client.h"
#include <mq/Plugin.h>
#include <string>
#include <windows.h>
#undef byte
#include <websocket.h>
#include <winhttp.h>

/**
 * @brief Constructor
 */
Client::Client() : m_hWebSocket(nullptr) {
	WriteChatf("MQ2Elixir: Client instance created.");
}

/**
 * @brief Destructor
 */
Client::~Client() {
	// Optionally, add automatic cleanup of m_hWebSocket if needed.
	WriteChatf("MQ2Elixir: Client instance destroyed.");
}

/**
 * @brief Sends a message to the server via the active WebSocket connection.
 * @param message The message to send.
 */
void Client::to_server(const std::string& message) {
	if (!m_hWebSocket) {
		WriteChatf("MQ2Elixir Error: No active WebSocket connection.");
		return;
	}
	if (send_message(message)) {
		WriteChatf("MQ2Elixir: Sent message to server.");
	}
	else {
		WriteChatf("MQ2Elixir Error: Failed to send message. Error: %d", GetLastError());
	}
}

/**
 * @brief Private helper function to send a message using WinHttpWebSocketSend.
 * @param message The message to send.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL Client::send_message(const std::string& message) {
	if (!m_hWebSocket) {
		WriteChatf("MQ2Elixir Error: No active WebSocket connection.");
		return FALSE;
	}
	DWORD bytesWritten = 0;
	return WinHttpWebSocketSend(m_hWebSocket, static_cast<_WINHTTP_WEB_SOCKET_BUFFER_TYPE>(2),
		(void*)message.c_str(), (DWORD)message.length());
}

/**
 * @brief Starts a WinHTTP session.
 * @param userAgent The user agent string.
 * @return HINTERNET session handle or nullptr on failure.
 */
HINTERNET Client::Start_Session(const std::string& userAgent) {
	std::wstring wUserAgent(userAgent.begin(), userAgent.end());
	HINTERNET hSession = WinHttpOpen(wUserAgent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) {
		WriteChatf("MQ2Elixir Error: Failed to start session. Error: %d", GetLastError());
	}
	return hSession;
}

/**
 * @brief Connects to the WebSocket server and joins the room:lobby channel.
 * @param hSession The WinHTTP session handle.
 * @param host The server host name or IP.
 * @param port The server port.
 * @return HINTERNET WebSocket handle or nullptr on failure.
 */
HINTERNET Client::Connect(HINTERNET hSession, const std::string& host, int port) {
	std::wstring wHost(host.begin(), host.end());
	HINTERNET hConnect = WinHttpConnect(hSession, wHost.c_str(), port, 0);
	if (!hConnect) {
		WriteChatf("MQ2Elixir Error: Failed to connect to server. Error: %d", GetLastError());
		return nullptr;
	}

	// Open a WebSocket request for the /ws endpoint (using insecure ws since WINHTTP_FLAG_SECURE is not set)
	std::wstring wsUrl = L"/ws";
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wsUrl.c_str(), NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
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

	m_hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, NULL);
	if (!m_hWebSocket) {
		WriteChatf("MQ2Elixir Error: Failed to complete WebSocket upgrade. Error: %d", GetLastError());
		WinHttpCloseHandle(hRequest);
		return nullptr;
	}

	// Join the room:lobby channel using the private send_message wrapper
	std::string joinMessage = "{\"topic\":\"room:lobby\",\"event\":\"phx_join\",\"payload\":{},\"ref\":null}";
	if (send_message(joinMessage)) {
		WriteChatf("MQ2Elixir: Joined room:lobby channel.");
	}
	else {
		WriteChatf("MQ2Elixir Error: Failed to join room:lobby. Error: %d", GetLastError());
	}

	return m_hWebSocket;
}

/**
 * @brief Cleans up WinHTTP handles.
 * @param hSession Session handle.
 * @param hConnect Connection handle.
 * @param hRequest Request handle.
 * @param hWebSocket WebSocket handle.
 */
void Client::Cleanup(HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest, HINTERNET hWebSocket) {
	if (hWebSocket) WinHttpCloseHandle(hWebSocket);
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	WriteChatf("MQ2Elixir: Cleanup complete.");
}
