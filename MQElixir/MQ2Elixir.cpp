#include <mq/Plugin.h>
#include <windows.h>
#include <winhttp.h>
#include <websocket.h>
#include <string>

#pragma comment(lib, "winhttp.lib")

#ifndef WINHTTP_WEB_SOCKET_UTF8_MESSAGE
#define WINHTTP_WEB_SOCKET_UTF8_MESSAGE 0x80000002
#endif

PreSetup("MQ2Elixir");
PLUGIN_VERSION(0.1);

HINTERNET hSession = NULL;
HINTERNET hConnect = NULL;
HINTERNET hWebSocket = NULL;

PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Elixir::Initializing version %f", MQ2Version);

	// Open an HTTP session
	hSession = WinHttpOpen(L"MQ2Elixir WebSocket Client/1.0",
		WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession) {
		WriteChatf("MQ2Elixir: Failed to open HTTP session. Error: %lu", GetLastError());
		return;
	}

	// Establish a connection to the WebSocket server
	hConnect = WinHttpConnect(hSession, L"10.0.0.9", 4000, 0);

	std::wstring requestUrl = L"/socket/websocket?vsn=2.0.0";
	HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", requestUrl.c_str(),
		NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);


	if (!hConnect) {
		WriteChatf("MQ2Elixir: Failed to connect to server. Error: %lu", GetLastError());
		WinHttpCloseHandle(hSession);
		hSession = NULL;
		return;
	}

	// Add WebSocket-specific headers with version and key
	WinHttpAddRequestHeaders(hRequest, L"Upgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n", (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD);

	// Send the request
	BOOL bSendRequest = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	if (!bSendRequest) {
		WriteChatf("MQ2Elixir: Failed to send WebSocket request. Error: %lu", GetLastError());
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	// Receive the response
	BOOL bReceiveResponse = WinHttpReceiveResponse(hRequest, NULL);

	if (!bReceiveResponse) {
		WriteChatf("MQ2Elixir: Failed to receive WebSocket response. Error: %lu", GetLastError());
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	// Upgrade to WebSocket protocol
	hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, NULL);
	WinHttpCloseHandle(hRequest);

	if (!hWebSocket) {
		WriteChatf("MQ2Elixir: Failed to upgrade to WebSocket. Error: %lu", GetLastError());
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}

	WriteChatf("MQ2Elixir: WebSocket connection established!");

	// Send character name to server
	char characterName[64] = "Unknown";
	if (pLocalPlayer && pLocalPlayer->Name)
		strcpy_s(characterName, pLocalPlayer->Name);

	std::string message = R"(["1", "1", "room:lobby", "phx_join", {}])";
	WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType = WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
	DWORD fmessageLength = static_cast<DWORD>(message.length());

	WinHttpWebSocketSend(hWebSocket, bufferType, (void*)&message[0], fmessageLength);
	//std::string fmessage = R"([
    //  "2",
    //  "2",
    //  "room:lobby",
    //  "new_msg",
    //  {"body": "Hello from C++!"},
    //  {}
    //])";
	WINHTTP_WEB_SOCKET_BUFFER_TYPE fbufferType = WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
	
	std::string simpleMessage = R"(["2", "2", "room:lobby", "new_msg", {"body": "Hello"}, {}])";
	DWORD simpleMessageLength = static_cast<DWORD>(simpleMessage.size());
	// Log the message to verify it's correct
	WriteChatf("Sending message : %s" , simpleMessage);
	DWORD result = WinHttpWebSocketSend(hWebSocket, fbufferType, simpleMessage.data(), simpleMessageLength);
	if (result == NO_ERROR) {
		WriteChatf("simplemessage Message sent successfully!", simpleMessage);
	}
	else {
		WriteChatf("Failed to send message. Error: %lu" ,GetLastError());
	}
	
}


PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Elixir::Shutting down");

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
}
