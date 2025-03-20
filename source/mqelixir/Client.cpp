#include "Client.h"

Client::Client()
    : hSession(nullptr), hConnection(nullptr), hWebSocket(nullptr) {}

Client::~Client() {
    disconnect();
}

void Client::connect(const std::string& serverAddress, int port) {
    std::wstring wServerAddress(serverAddress.begin(), serverAddress.end());

    hSession = WinHttpOpen(L"Client/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::cerr << "WinHttpOpen failed with error: " << GetLastError() << std::endl;
        return;
    }

    hConnection = WinHttpConnect(hSession, wServerAddress.c_str(), port, 0);
    if (!hConnection) {
        std::cerr << "WinHttpConnect failed with error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession);
        return;
    }

    hWebSocket = WinHttpWebSocketCompleteUpgrade(hConnection, 0);
    if (!hWebSocket) {
        std::cerr << "WebSocket upgrade failed with error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hConnection);
        WinHttpCloseHandle(hSession);
        return;
    }

    std::cout << "Connected to " << serverAddress << " on port " << port << std::endl;
}

void Client::join(const std::string& channel) {
    std::string joinMessage = "JOIN " + channel;
    DWORD dwBytesWritten = 0;
    WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType = WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE;

    int result = WinHttpWebSocketSend(hWebSocket, bufferType,
                                      (void*)joinMessage.c_str(), joinMessage.size());
    if (result != NO_ERROR) {
        std::cerr << "Failed to send join message, error: " << result << std::endl;
    } else {
        std::cout << "Joined channel: " << channel << std::endl;
    }
}

void Client::disconnect() {
    if (hWebSocket) {
        WinHttpWebSocketClose(hWebSocket, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
        WinHttpCloseHandle(hWebSocket);
        hWebSocket = nullptr;
    }
    if (hConnection) {
        WinHttpCloseHandle(hConnection);
        hConnection = nullptr;
    }
    if (hSession) {
        WinHttpCloseHandle(hSession);
        hSession = nullptr;
    }

    std::cout << "Disconnected from server." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <command> [arguments]" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    Client client;

    if (command == "connect" && argc == 4) {
        std::string serverAddress = argv[2];
        int port = std::stoi(argv[3]);
        client.connect(serverAddress, port);
    } else if (command == "join" && argc == 3) {
        std::string channel = argv[2];
        client.join(channel);
    } else if (command == "disconnect") {
        client.disconnect();
    } else {
        std::cerr << "Unknown command or incorrect arguments." << std::endl;
        return 1;
    }

    return 0;
}
