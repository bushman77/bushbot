// Elixir.cpp - MQ2Elixir Plugin using TCP (no WebSocket)
// Author: you 😎

#include "MQ2Plugin.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

PreSetup("MQ2Elixir");

PLUGIN_VERSION(1.0);

void SendToElixirServer(const std::string& message) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    sockaddr_in serverAddr;
    char recvbuf[512];
    int recvbuflen = sizeof(recvbuf);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        WriteChatf("[MQ2Elixir] WSAStartup failed.");
        return;
    }

    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        WriteChatf("[MQ2Elixir] Socket creation failed.");
        WSACleanup();
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4040);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(ConnectSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        WriteChatf("[MQ2Elixir] Failed to connect to Elixir server.");
        closesocket(ConnectSocket);
        WSACleanup();
        return;
    }

    send(ConnectSocket, message.c_str(), static_cast<int>(message.length()), 0);

    int bytesReceived = recv(ConnectSocket, recvbuf, recvbuflen - 1, 0);
    if (bytesReceived > 0) {
        recvbuf[bytesReceived] = '\0';
        WriteChatf("[MQ2Elixir] Received from Elixir: %s", recvbuf);
    } else {
        WriteChatf("[MQ2Elixir] No response from Elixir server.");
    }

    closesocket(ConnectSocket);
    WSACleanup();
}

void ElixirCommand(PSPAWNINFO pChar, PCHAR szLine) {
    if (!szLine || !*szLine) {
        WriteChatf("[MQ2Elixir] Usage: /elixir <message>");
        return;
    }

    std::string message(szLine);
    message += "\n"; // Ensure newline termination
    SendToElixirServer(message);
}

PLUGIN_API void InitializePlugin() {
    AddCommand("/elixir", ElixirCommand);
    WriteChatf("[MQ2Elixir] Plugin loaded.");
}

PLUGIN_API void ShutdownPlugin() {
    RemoveCommand("/elixir");
    WriteChatf("[MQ2Elixir] Plugin unloaded.");
}
