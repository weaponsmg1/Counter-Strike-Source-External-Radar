#include "memory.h"
#include <iostream>

Memory::~Memory() {
    if (hProcess_) CloseHandle(hProcess_);
}

bool Memory::AttachProcess(const std::wstring& processName) {
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    bool found = false;
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0) {
                pid_ = pe32.th32ProcessID;
                hProcess_ = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid_);
                found = hProcess_ != nullptr;
                break;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return found;
}

Memory::Module Memory::GetModule(const std::wstring& moduleName) {
    MODULEENTRY32W me32;
    me32.dwSize = sizeof(MODULEENTRY32W);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid_);
    if (hSnapshot == INVALID_HANDLE_VALUE) return Module();

    Module result;
    if (Module32FirstW(hSnapshot, &me32)) {
        do {
            if (_wcsicmp(me32.szModule, moduleName.c_str()) == 0) {
                result.base = reinterpret_cast<uintptr_t>(me32.modBaseAddr);
                result.size = me32.modBaseSize;
                result.found = true;
                break;
            }
        } while (Module32NextW(hSnapshot, &me32));
    }

    CloseHandle(hSnapshot);
    return result;
}