#pragma once
#include <windows.h>
#include <string>

bool SendWebSocketMessage(HINTERNET hWebSocket, const std::string& message);
