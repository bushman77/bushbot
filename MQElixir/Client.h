#pragma once

#include <windows.h>
#include <winhttp.h>
#include <string>

/**
 * @brief The Client class encapsulates WebSocket connection management.
 *
 * This class provides methods to start a WinHTTP session, connect to a WebSocket server,
 * send messages to the server, and clean up resources.
 */
class Client
{
public:
	Client();
	~Client(); // Destructor for any future automatic cleanup if needed.

	/**
	 * @brief Sends a message to the server.
	 * @param message The message to send.
	 */
	void to_server(const std::string& message); // Previously named Broadcast

	/**
	 * @brief Starts a WinHTTP session.
	 * @param userAgent The user agent string.
	 * @return HINTERNET session handle or nullptr on failure.
	 */
	HINTERNET Start_Session(const std::string& userAgent);

	/**
	 * @brief Connects to the WebSocket server and joins the room:lobby channel.
	 * @param hSession The WinHTTP session handle.
	 * @param host The server host name or IP.
	 * @param port The server port.
	 * @return HINTERNET WebSocket handle or nullptr on failure.
	 */
	HINTERNET Connect(HINTERNET hSession, const std::string& host, int port);

	/**
	 * @brief Cleans up WinHTTP handles.
	 * @param hSession Session handle.
	 * @param hConnect Connection handle.
	 * @param hRequest Request handle.
	 * @param hWebSocket WebSocket handle.
	 */
	void Cleanup(HINTERNET hSession, HINTERNET hConnect, HINTERNET hRequest, HINTERNET hWebSocket);
};
