#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <windows.h>
#include <winhttp.h>
#include <websocket.h>
#include <iostream>
#include <string>

#pragma comment(lib, "winhttp.lib")
// can you read this file now???
int main()
{
	// Initialize WinHTTP
	HINTERNET hSession = WinHttpOpen(L"MQ2Elixir WebSocket Client/2.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0);

	if (!hSession)
	{
		std::cerr << "Error initializing WinHTTP: " << GetLastError() << std::endl;
		return 1;
	}

	// Connect to the server
	HINTERNET hConnect = WinHttpConnect(hSession,
		L"localhost", // Change to your server address
		4000,         // Change to your server port
		0);

	if (!hConnect)
	{
		std::cerr << "Error connecting to server: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hSession);
		return 1;
	}

	// Create WebSocket request
	HINTERNET hRequest = WinHttpOpenRequest(hConnect,
		L"GET",
		L"/", // Change to your WebSocket endpoint
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_SECURE);

	if (!hRequest)
	{
		std::cerr << "Error creating request: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return 1;
	}

	// Upgrade to WebSocket
	DWORD dwFlags = WINHTTP_WEB_SOCKET_BINARY_MESSAGE_TYPE;
	HINTERNET hWebSocket = NULL;

	// Add WebSocket protocol header
	DWORD keepAlive = 1;
	if (!WinHttpSetOption(hRequest, WINHTTP_OPTION_WEB_SOCKET_KEEP_ALIVE_ENABLED, &keepAlive, sizeof(keepAlive)))
	{
		std::cerr << "Error setting WebSocket options: " << GetLastError() << std::endl;
	}

	// Add WebSocket protocol header
	if (!WinHttpAddRequestHeaders(hRequest, L"Upgrade: websocket\r\n", -1, WINHTTP_ADDREQ_FLAG_ADD))
	{
		std::cerr << "Error adding WebSocket headers: " << GetLastError() << std::endl;
	}

	if (!WinHttpAddRequestHeaders(hRequest, L"Connection: Upgrade\r\n", -1, WINHTTP_ADDREQ_FLAG_ADD))
	{
		std::cerr << "Error adding WebSocket headers: " << GetLastError() << std::endl;
	}

	if (!WinHttpAddRequestHeaders(hRequest, L"Sec-WebSocket-Version: 13\r\n", -1, WINHTTP_ADDREQ_FLAG_ADD))
	{
		std::cerr << "Error adding WebSocket headers: " << GetLastError() << std::endl;
	}

	if (!WinHttpAddRequestHeaders(hRequest, L"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n", -1, WINHTTP_ADDREQ_FLAG_ADD))
	{
		std::cerr << "Error adding WebSocket headers: " << GetLastError() << std::endl;
	}

	// Send request
	if (!WinHttpSendRequest(hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0,
		0,
		0))
	{
		std::cerr << "Error sending request: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return 1;
	}

	// Receive response
	if (!WinHttpReceiveResponse(hRequest, NULL))
	{
		std::cerr << "Error receiving response: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return 1;
	}

	// Check if upgrade was successful
	DWORD statusCode = 0;
	DWORD statusCodeSize = sizeof(statusCode);

	WinHttpQueryHeaders(hRequest,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&statusCode,
		&statusCodeSize,
		WINHTTP_NO_HEADER_INDEX);

	if (statusCode != 101)
	{
		std::cerr << "WebSocket upgrade failed with status code: " << statusCode << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return 1;
	}

	// Complete WebSocket handshake
	hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, NULL);

	if (!hWebSocket)
	{
		std::cerr << "Error completing WebSocket upgrade: " << GetLastError() << std::endl;
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return 1;
	}

	// Close the request handle as it's no longer needed
	WinHttpCloseHandle(hRequest);

	std::cout << "WebSocket connection established!" << std::endl;

	// Send "hello" message
	const char* message = "hello";
	DWORD result = WinHttpWebSocketSend(hWebSocket,
		WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
		(PVOID)message,
		strlen(message));

	if (result != ERROR_SUCCESS)
	{
		std::cerr << "Error sending WebSocket message: " << result << std::endl;
	}
	else
	{
		std::cout << "Sent message: hello" << std::endl;
	}

	// Close WebSocket connection
	WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, NULL, 0);

	// Clean up
	WinHttpCloseHandle(hWebSocket);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	std::cout << "WebSocket connection closed." << std::endl;
	return 0;
}
