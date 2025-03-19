#include "Client.h"
#include <iostream>
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

Client::Client() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }
#endif
}

Client::~Client() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void Client::connect(const std::string& serverAddress, int port) {
    int sock;
    struct sockaddr_in server;

#ifdef _WIN32
    sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        return;
    }
#else
    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return;
    }
#endif

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(serverAddress.c_str());

    if (::connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed." << std::endl;
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return;
    }

    std::cout << "Connected to " << serverAddress << " on port " << port << std::endl;

#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

void Client::join(const std::string& channel) {
    // Implement join logic
    std::cout << "Joining channel " << channel << std::endl;
}

void Client::disconnect() {
    // Implement disconnection logic
    std::cout << "Disconnecting" << std::endl;
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
