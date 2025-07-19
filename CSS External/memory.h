#pragma once
#include <windows.h>
#include <string>
#include <tlhelp32.h>

class Memory {
public:
    struct Module {
        uintptr_t base = 0; 
        size_t size = 0;  
        bool found = false; 
    };

    Module base_client_;
    Module base_engine_;

    Memory() = default;
    ~Memory();

    bool AttachProcess(const std::wstring& processName);
    Module GetModule(const std::wstring& moduleName);

    template<typename T>
    T read(uintptr_t address) {
        T buffer;
        ReadProcessMemory(hProcess_, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), nullptr);
        return buffer;
    }

    template<typename T>
    bool write(uintptr_t address, const T& value) {
        return WriteProcessMemory(hProcess_, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
    }

    // Process ID
    DWORD pid_ = 0;
    DWORD GetPID() const { return pid_; }

private:
    HANDLE hProcess_ = nullptr;
};