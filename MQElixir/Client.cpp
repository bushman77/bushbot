#include "Client.h"
#include "MQ2Plugin.h"
#include <iostream>

Client::Client() : hSession(nullptr), hConnection(nullptr), hRequest(nullptr) {}

// Command map initialization
const std::unordered_map<std::string, std::function<void(Client*, const std::string&)>> Client::commandMap = {
	{"connect", [](Client* client, const std::string& arg) {
		auto delimiterPos = arg.find(':');
		if (delimiterPos != std::string::npos) {
			std::string ip = arg.substr(0, delimiterPos);
			int port = std::stoi(arg.substr(delimiterPos + 1));
			if (client->connect(ip, port)) {
				WriteChatf("Connected to %s:%d", ip.c_str(), port);
			}
 else {
  WriteChatf("Failed to connect to %s:%d", ip.c_str(), port);
}
}
else {
 WriteChatf("Invalid connect argument. Use format: <ip>:<port>");
}
}},
{"disconnect", [](Client* client, const std::string&) {
	client->disconnect();
	WriteChatf("Disconnected");
}},
{"send", [](Client* client, const std::string& message) {
	client->sendMessage(message);
}}
};

// Request handler
bool Client::request(const std::string& command, const std::string& arg) {
	auto it = commandMap.find(command);
	if (it != commandMap.end()) {
		it->second(this, arg);
		return true;
	}
	WriteChatf("Unknown command: %s", command.c_str());
	return false;
}

// Connection function
bool Client::connect(const std::string& ip, int port) {
	hSession = WinHttpOpen(L"MQ2Elixir/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, nullptr, nullptr, 0);
	if (!hSession) return false;

	std::wstring wip(ip.begin(), ip.end());
	hConnection = WinHttpConnect(hSession, wip.c_str(), port, 0);
	if (!hConnection) {
		WinHttpCloseHandle(hSession);
		return false;
	}

	WriteChatf("Successfully connected to %s:%d", ip.c_str(), port);
	return true;
}

// Disconnect function
void Client::disconnect() {
	if (hRequest) {
		WinHttpCloseHandle(hRequest);
		hRequest = nullptr;
	}
	if (hConnection) {
		WinHttpCloseHandle(hConnection);
		hConnection = nullptr;
	}
	if (hSession) {
		WinHttpCloseHandle(hSession);
		hSession = nullptr;
	}
	WriteChatf("Disconnected successfully.");
}

// Send message function
void Client::sendMessage(const std::string& message) {
	WriteChatf("Sending message to server: %s", message.c_str());
	// Implementation of sending a message to the server
}
