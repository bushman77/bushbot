#pragma once

#include <winsock2.h>    // for SOCKET
#include <windows.h>     // for HWND, DWORD, etc.
#include <string>
#include "../MQ2Plugin.h"// for PSPAWNINFO, PCHAR, PLUGIN_API

// ──────────────────────────────────────────────────────────────
// Globals (defined in .cpp)
// ──────────────────────────────────────────────────────────────
extern SOCKET g_socket;

// ──────────────────────────────────────────────────────────────
// Internal helper prototypes (if you really need to expose them;
// otherwise you can keep these static in the .cpp)
// ──────────────────────────────────────────────────────────────
bool        IsActiveClient();
void        safe_close_socket();
std::string BuildCharacterJson();
void        HandleElixirMessage(const std::string& raw);
void        SetEQWindowTitle();

// ──────────────────────────────────────────────────────────────
// MacroQuest plugin hooks
// ──────────────────────────────────────────────────────────────
PLUGIN_API void InitializePlugin();
PLUGIN_API void ShutdownPlugin();

PLUGIN_API void ElixirCommand(PSPAWNINFO, PCHAR line);
PLUGIN_API void EcaCommand(PSPAWNINFO, PCHAR line);
PLUGIN_API void SwitchWindowCommand(PSPAWNINFO, PCHAR line);

PLUGIN_API void OnWriteChatColor(const char* line, DWORD color);
PLUGIN_API void OnPulse();
PLUGIN_API void OnZoned();
