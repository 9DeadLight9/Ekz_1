#include <windows.h>
#include <iostream>
#include <string>

void ErrorExit(const char* msg) {
    std::cerr << msg << " (Error: " << GetLastError() << ")" << std::endl;
    ExitProcess(1);
}

int main() {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        ErrorExit("Failed to create pipe");
    }

    if (!SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0)) {
        ErrorExit("Failed to set handle information");
    }

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.hStdInput = hReadPipe;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags |= STARTF_USESTDHANDLES;

    const std::wstring command = L"notepad.exe";

    if (!CreateProcessW(
        NULL, 
        const_cast<LPWSTR>(command.c_str()),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi)) {
        ErrorExit("Failed to create child process");
    }

    CloseHandle(hReadPipe);

    const std::string message = "Hello from parent process!\n";
    DWORD bytesWritten;
    if (!WriteFile(hWritePipe, message.c_str(), message.size(), &bytesWritten, NULL)) {
        ErrorExit("Failed to write to pipe");
    }

    CloseHandle(hWritePipe);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
