#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <windows.h>
#include <winhttp.h>
#include "MQ2Plugin.h"
class Client {
  public:
    Client();
      bool request(const std::string& command, const std::string& arg = "");

  private:
    bool connect(const std::string& ip, int port);
    void disconnect();
    void sendMessage(const std::string& message);

    HINTERNET hSession;
    HINTERNET hConnection;
    HINTERNET hRequest;

    static const std::unordered_map<std::string, std::function<void(Client*, const std::string&)>> commandMap;
};
