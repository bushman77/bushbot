// MQ2Elixir.cpp : Defines the entry point for the DLL application.
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define _WINSOCKAPI_ // Prevents `winsock.h` from being included

#include <mq/Plugin.h>
#include <winsock2.h>
#include <windows.h>
#include <boost/asio.hpp>
#include <iostream>
#include <fmt/core.h>
#include <fmt/format.h>

// Disable warning 4267 for WebSocket++ (size_t to smaller type conversions)
#pragma warning(push)
#pragma warning(disable : 4267)
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#pragma warning(pop)

// Set up plugin metadata
PreSetup("MQ2Elixir");
PLUGIN_VERSION(0.1);

// Use the non-TLS config
using WebSocketClient = websocketpp::client<websocketpp::config::asio_client>;
using websocketpp::connection_hdl;

// Global WebSocket client and connection handle
WebSocketClient wsClient;
connection_hdl wsHandle;

// Point this to your Phoenix server's WebSocket endpoint
std::string serverUri = "ws://127.0.0.1:4000/socket/websocket";

//------------------------------------------------------------------------------
// ConnectToElixir: Attempts a WebSocket connection to your Elixir/Phoenix server
//------------------------------------------------------------------------------
void ConnectToElixir()
{
	try {
		// Initialize the ASIO transport
		wsClient.init_asio();

		// Called when the connection is successfully established
		wsClient.set_open_handler([&](connection_hdl hdl) {
			wsHandle = hdl;
			WriteChatf("\ar[MQ2Elixir] \agConnected to Elixir GenServer!");
			});

		// Called when the connection fails
		wsClient.set_fail_handler([&](connection_hdl hdl) {
			WriteChatf("\ar[MQ2Elixir] \aoFailed to connect to Elixir GenServer!");
			});

		// Create a new connection
		websocketpp::lib::error_code ec;
		auto conn = wsClient.get_connection(serverUri, ec);
		if (ec) {
			WriteChatf("\ar[MQ2Elixir] \aoConnection error: %s", ec.message().c_str());
			return;
		}

		// Queue the connection
		wsClient.connect(conn);

		// Run the WebSocket event loop in a separate thread
		std::thread([&]() {
			wsClient.run();
			}).detach();
	}
	catch (const std::exception& e) {
		WriteChatf("\ar[MQ2Elixir] \aoException: %s", e.what());
	}
}

//------------------------------------------------------------------------------
// ElixirCommand: Handles /elixir commands
//------------------------------------------------------------------------------
VOID ElixirCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if (stricmp(szLine, "connect") == 0) {
		WriteChatf("\ar[MQ2Elixir] \awAttempting to connect to Elixir GenServer...");
		ConnectToElixir();
	}
	else {
		WriteChatf("\ar[MQ2Elixir] \awUsage: /elixir connect");
	}
}

//------------------------------------------------------------------------------
// InitializePlugin / ShutdownPlugin: Called when the plugin is loaded/unloaded
//------------------------------------------------------------------------------
PLUGIN_API void InitializePlugin()
{
	DebugSpewAlways("MQ2Elixir::Initializing version %f", MQ2Version);
	AddCommand("/elixir", ElixirCommand);
}

PLUGIN_API void ShutdownPlugin()
{
	DebugSpewAlways("MQ2Elixir::Shutting down");
	RemoveCommand("/elixir");
}

//------------------------------------------------------------------------------
// Below are MQ2 callback stubs you can remove or expand as needed
//------------------------------------------------------------------------------

PLUGIN_API void OnCleanUI() {}
PLUGIN_API void OnReloadUI() {}
PLUGIN_API void OnDrawHUD() {}
PLUGIN_API void SetGameState(int GameState) {}
PLUGIN_API void OnPulse() {}
PLUGIN_API void OnWriteChatColor(const char* Line, int Color, int Filter) {}
PLUGIN_API bool OnIncomingChat(const char* Line, DWORD Color) { return false; }
PLUGIN_API void OnAddSpawn(PSPAWNINFO pNewSpawn) {}
PLUGIN_API void OnRemoveSpawn(PSPAWNINFO pSpawn) {}
PLUGIN_API void OnAddGroundItem(PGROUNDITEM pNewGroundItem) {}
PLUGIN_API void OnRemoveGroundItem(PGROUNDITEM pGroundItem) {}
PLUGIN_API void OnBeginZone() {}
PLUGIN_API void OnEndZone() {}
PLUGIN_API void OnZoned() {}
PLUGIN_API void OnUpdateImGui() {}
PLUGIN_API void OnMacroStart(const char* Name) {}
PLUGIN_API void OnMacroStop(const char* Name) {}
PLUGIN_API void OnLoadPlugin(const char* Name) {}
PLUGIN_API void OnUnloadPlugin(const char* Name) {}
