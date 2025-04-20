#include "../MQ2Plugin.h"
#include <windows.h>
#include <string>
#include <chrono>
#include <memory>

PreSetup("MQ2Elixir");

namespace {
	// Constants
	constexpr int DEFAULT_TIMEOUT_MS = 3000;
	constexpr int PIPE_BUFFER_SIZE = 4096;
	const std::string DEFAULT_SERVER_PATH = "C:\\release\\server";

	// Process handles wrapper for RAII
	struct ProcessHandles {
		HANDLE hProcess = NULL;
		HANDLE hInputRead = NULL;
		HANDLE hInputWrite = NULL;
		HANDLE hOutputRead = NULL;
		HANDLE hOutputWrite = NULL;

		~ProcessHandles() {
			if (hProcess) TerminateProcess(hProcess, 0);
			if (hProcess) CloseHandle(hProcess);
			if (hInputRead) CloseHandle(hInputRead);
			if (hInputWrite) CloseHandle(hInputWrite);
			if (hOutputRead) CloseHandle(hOutputRead);
			if (hOutputWrite) CloseHandle(hOutputWrite);
		}

		bool isRunning() const {
			if (!hProcess) return false;
			DWORD exitCode;
			return GetExitCodeProcess(hProcess, &exitCode) && exitCode == STILL_ACTIVE;
		}
	};

	// Global process instance
	std::unique_ptr<ProcessHandles> g_process;
}

bool CreatePipes(ProcessHandles& ph) {
	SECURITY_ATTRIBUTES saAttr = {
		sizeof(SECURITY_ATTRIBUTES),
		NULL,
		TRUE  // Handles are inheritable
	};

	// Create stdout pipe
	if (!CreatePipe(&ph.hOutputRead, &ph.hOutputWrite, &saAttr, 0) ||
		!SetHandleInformation(ph.hOutputRead, HANDLE_FLAG_INHERIT, 0)) {
		WriteChatf("[MQ2Elixir] Error creating output pipe: %d", GetLastError());
		return false;
	}

	// Create stdin pipe
	if (!CreatePipe(&ph.hInputRead, &ph.hInputWrite, &saAttr, 0) ||
		!SetHandleInformation(ph.hInputWrite, HANDLE_FLAG_INHERIT, 0)) {
		WriteChatf("[MQ2Elixir] Error creating input pipe: %d", GetLastError());
		return false;
	}

	return true;
}

static std::string ReadFromElixir(int timeoutMs = DEFAULT_TIMEOUT_MS);

bool StartElixir(const std::string& serverPath = DEFAULT_SERVER_PATH) {
	if (g_process && g_process->isRunning()) {
		WriteChatf("[MQ2Elixir] Elixir is already running");
		return true;
	}

	g_process = std::make_unique<ProcessHandles>();

	if (!CreatePipes(*g_process)) {
		g_process.reset();
		return false;
	}

	STARTUPINFOA si = { sizeof(STARTUPINFOA) };
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = g_process->hInputRead;
	si.hStdOutput = g_process->hOutputWrite;
	si.hStdError = g_process->hOutputWrite;

	PROCESS_INFORMATION pi = { 0 };

	std::string command = "cmd /C cd /d \"" + serverPath + "\" && mix run --no-halt";
	WriteChatf("[MQ2Elixir] Starting Elixir: %s", command.c_str());

	if (!CreateProcessA(
		NULL,
		(LPSTR)command.c_str(),
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		DWORD error = GetLastError();
		WriteChatf("[MQ2Elixir] Failed to start Elixir process: %d", error);
		g_process.reset();
		return false;
	}

	g_process->hProcess = pi.hProcess;
	CloseHandle(pi.hThread);

	// Wait for initialization
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Read initial message
	std::string initMsg = ReadFromElixir(2000);
	if (!initMsg.empty()) {
		WriteChatf("[Elixir] %s", initMsg.c_str());
	}

	WriteChatf("[MQ2Elixir] Elixir started (PID: %d)", pi.dwProcessId);
	return true;
}

void StopElixir() {
	g_process.reset();
}

bool SendToElixir(const std::string& cmd) {
	if (!g_process || !g_process->isRunning()) {
		WriteChatf("[MQ2Elixir] Cannot send command - Elixir not running");
		return false;
	}

	std::string fullCmd = cmd + "\n";
	DWORD bytesWritten;

	if (!WriteFile(g_process->hInputWrite, fullCmd.c_str(),
		static_cast<DWORD>(fullCmd.length()), &bytesWritten, NULL)) {
		WriteChatf("[MQ2Elixir] Error writing to Elixir: %d", GetLastError());
		StopElixir();
		return false;
	}

	FlushFileBuffers(g_process->hInputWrite);
	return true;
}

static std::string ReadFromElixir(int timeoutMs) {
	if (!g_process) return "";

	std::string result;
	char buffer[PIPE_BUFFER_SIZE];
	DWORD bytesRead;
	auto startTime = std::chrono::steady_clock::now();

	while (std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - startTime).count() < timeoutMs) {

		if (PeekNamedPipe(g_process->hOutputRead, NULL, 0, NULL, &bytesRead, NULL) && bytesRead > 0) {
			if (ReadFile(g_process->hOutputRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
				buffer[bytesRead] = '\0';
				result += buffer;

				if (result.find('\n') != std::string::npos) {
					break;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Clean response
	size_t pos = result.find('\n');
	if (pos != std::string::npos) {
		result = result.substr(0, pos);
	}
	result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());

	return result;
}

static void ElixirCommand(PSPAWNINFO pChar, PCHAR szLine) {
	if (!g_process || !g_process->isRunning()) {
		WriteChatf("[MQ2Elixir] Elixir is not running. Attempting to restart...");
		if (!StartElixir()) {
			WriteChatf("[MQ2Elixir] Failed to restart Elixir");
			return;
		}
	}

	if (!SendToElixir(szLine)) return;

	std::string reply = ReadFromElixir();
	if (reply.empty()) {
		WriteChatf("[MQ2Elixir] No response from Elixir (timeout)");
	}
	else {
		WriteChatf("[Elixir] %s", reply.c_str());
	}
}

PLUGIN_API VOID InitializePlugin() {
	DebugSpewAlways("MQ2Elixir::InitializePlugin()");
	AddCommand("/elixir", ElixirCommand);
	StartElixir();
}

PLUGIN_API VOID ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin()");
	RemoveCommand("/elixir");
	StopElixir();
}