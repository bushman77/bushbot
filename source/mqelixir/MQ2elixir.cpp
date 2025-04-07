#include "../MQ2Plugin.h"
#include <windows.h>
#include <string>
#include <iostream>

PreSetup("MQ2Elixir");

HANDLE hProcess = NULL;
HANDLE hInputWrite = NULL;
HANDLE hOutputRead = NULL;

bool StartElixirRelease(const std::string& releasePath) {
	SECURITY_ATTRIBUTES saAttr{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

	HANDLE hInputRead = NULL;
	HANDLE hOutputWrite = NULL;

	// Create pipes for Elixir stdin and stdout
	if (!CreatePipe(&hOutputRead, &hOutputWrite, &saAttr, 0)) return false;
	if (!CreatePipe(&hInputRead, &hInputWrite, &saAttr, 0)) return false;

	STARTUPINFOA si{};
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = hInputRead;
	si.hStdOutput = hOutputWrite;
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	PROCESS_INFORMATION pi{};

	std::string cmd = releasePath + "\\bin\\mq2elixir.bat start";

	BOOL success = CreateProcessA(
		NULL, (LPSTR)cmd.c_str(), NULL, NULL, TRUE,
		CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	if (!success) return false;

	hProcess = pi.hProcess;
	CloseHandle(pi.hThread);
	return true;
}

void StopElixirRelease() {
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
	WriteFile(hInputWrite, "\n", 1, &written, NULL); // newline triggers processing
}

std::string ReadFromElixir() {
	char buffer[512];
	DWORD bytesRead;
	std::string result;

	// Read until newline or timeout
	while (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
		buffer[bytesRead] = '\0';
		result += buffer;

		if (result.find('\n') != std::string::npos) break;
	}

	// Remove newline
	size_t pos = result.find('\n');
	if (pos != std::string::npos) result = result.substr(0, pos);

	return result;
}

void ElixirCommand(PSPAWNINFO pChar, PCHAR szLine) {
	if (!hProcess) {
		WriteChatf("[MQ2Elixir] Elixir process is not running.");
		return;
	}

	std::string cmd = szLine;
	SendToElixir(cmd);
	std::string reply = ReadFromElixir();

	if (reply.empty()) {
		WriteChatf("[MQ2Elixir] No response from Elixir.");
	}
	else {
		WriteChatf("[Elixir] %s", reply.c_str());
	}
}

PLUGIN_API VOID InitializePlugin() {
	DebugSpewAlways("MQ2Elixir::InitializePlugin()");
	AddCommand("/elixir", ElixirCommand);

	std::string elixirPath = "C:\\macroquest\\plugins";
	std::string cmd = elixirPath + "\\mq2elixir.bat start";


	if (StartElixirRelease(elixirPath)) {
		WriteChatf("[MQ2Elixir] Elixir release started.");
	}
	else {
		WriteChatf("[MQ2Elixir] Failed to start Elixir release.");
	}
}

PLUGIN_API VOID ShutdownPlugin() {
	DebugSpewAlways("MQ2Elixir::ShutdownPlugin()");
	RemoveCommand("/elixir");

	StopElixirRelease();
	WriteChatf("[MQ2Elixir] Elixir release stopped.");
}
