#include "../MQ2Plugin.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <thread>

PreSetup("MQ2Elixir");

HANDLE hProcess = NULL;
HANDLE hInputWrite = NULL;
HANDLE hOutputRead = NULL;

bool IsElixirRunning() {
	return hProcess != NULL;
}

bool StartElixir(const std::string& serverPath) {
	SECURITY_ATTRIBUTES saAttr{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	HANDLE hInputRead = NULL;
	HANDLE hOutputWrite = NULL;

	if (!CreatePipe(&hOutputRead, &hOutputWrite, &saAttr, 0)) return false;
	if (!CreatePipe(&hInputRead, &hInputWrite, &saAttr, 0)) return false;

	STARTUPINFOA si{};
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hInputRead;
	si.hStdOutput = hOutputWrite;
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	PROCESS_INFORMATION pi{};

	std::string cmd = "cmd /C cd /d " + serverPath + " && elixir server.exs";

	BOOL success = CreateProcessA(
		NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE,
		CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	if (!success) return false;

	hProcess = pi.hProcess;
	CloseHandle(pi.hThread);
	return true;
}

void StopElixir() {
	if (hProcess) {
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
		hProcess = NULL;
	}
	if (hInputWrite) {
		CloseHandle(hInputWrite);
		hInputWrite = NULL;
	}
	if (hOutputRead) {
		CloseHandle(hOutputRead);
		hOutputRead = NULL;
	}
}

void SendToElixir(const std::string& cmd) {
	DWORD written;
	WriteFile(hInputWrite, cmd.c_str(), cmd.length(), &written, NULL);
	WriteFile(hInputWrite, "\n", 1, &written, NULL);
}

std::string ReadFromElixir(int timeoutMs = 1000) {
	std::string result;
	char buffer[512];
	DWORD bytesRead;
	DWORD totalWait = 0;
	const DWORD step = 50;

	while (totalWait < timeoutMs) {
		if (PeekNamedPipe(hOutputRead, NULL, 0, NULL, &bytesRead, NULL) && bytesRead > 0) {
			if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
				buffer[bytesRead] = '\0';
				result += buffer;
				if (result.find('\n') != std::string::npos) break;
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(step));
			totalWait += step;
		}
	}

	size_t pos = result.find('\n');
	if (pos != std::string::npos) result = result.substr(0, pos);

	return result;
}

void ElixirCommand(PSPAWNINFO pChar, PCHAR szLine) {
	if (!IsElixirRunning()) {
		WriteChatf("[MQ2Elixir] Elixir is not running.");
		return;
	}

	std::string cmd = szLine;
	SendToElixir(cmd);
	std::string reply = ReadFromElixir();

	if (reply.empty()) {
		WriteChatf("[MQ2Elixir] No response from Elixir (timeout).");
	}
	else {
		WriteChatf("[Elixir] %s", reply.c_str());
	}
}

PLUGIN_API VOID InitializePlugin() {
	DebugSpewAlways("MQ2Elixir::InitializePlugin()");
	AddCommand("/elixir", ElixirCommand);

	std::string serverPath = "C:\\release\\server";

	if (StartElixir(serverPath)) {
		WriteChatf("[MQ2Elixir] Elixir started.");
	}
	else {
		WriteChatf("[MQ2Elixir] Failed to start Elixir.");
	}
}

PLUGIN_API VOID ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin()");
	RemoveCommand("/elixir");

	StopElixir();
	WriteChatf("[MQ2Elixir] Elixir stopped.");
}
