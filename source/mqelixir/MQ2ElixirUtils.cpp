#include <mq/Plugin.h>
#include <string>

HINTERNET hSession = NULL;
HINTERNET hConnect = NULL;
HINTERNET hWebSocket = NULL;


// Utility function to send a message over WebSocket
bool SendWebSocketMessage(HINTERNET hWebSocket, const std::string& message) {
	WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType = WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
	DWORD messageLength = static_cast<DWORD>(message.length());

	DWORD result = WinHttpWebSocketSend(hWebSocket, bufferType, (void*)message.c_str(), messageLength);
	if (result == NO_ERROR) {
		WriteChatf("Message sent successfully: %s", message.c_str());
		return true;
	}
	else {
		WriteChatf("Failed to send message. Error: %lu", GetLastError());
		return false;
	}
}
