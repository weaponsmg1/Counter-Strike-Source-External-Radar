#include <iostream>
#include <windows.h>
#include <string>
#include "offsets.h"
#include <random>
#include <sstream>

#include "Overlay.h"
#include "config.h"
#include "memory.h"
#include "vector.h"

const int radarSize = 250;      
const int radarPosX = 20;      
const int radarPosY = 20;      
const float radarScale = 0.1f;

#define M_PI 3.14159265358979323846

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN && wParam == VK_INSERT) {
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0,
                      hInstance, NULL, NULL, NULL, NULL,
                      L"OverlayWindow", NULL };
    RegisterClassEx(&wc);

  

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName, L"Overlay",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, wc.hInstance, NULL);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    Overlay overlay(hwnd);

    Memory memory;
    bool process = false;
    bool dll = false;

    while (!process) {
        if (memory.AttachProcess(L"cstrike_win64.exe")) {
            process = true;
            std::wcout << L"Css found PID: " << memory.pid_ << std::endl;
        }
        else {
            std::wcout << L"cstrike_win64.exe not found waiting..." << std::endl;
            Sleep(1000);
        }
    }

    while (!dll) {
        memory.base_client_ = memory.GetModule(L"client.dll");
        if (memory.base_client_.found) {
            dll = true;
            std::wcout << L"client.dll found" << std::endl;
        }
        else {
            std::wcout << L"client.dll not found waiting..." << std::endl;
            Sleep(1000);
        }
    }

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        overlay.BeginDraw();

        overlay.DrawFilledBox(radarPosX, radarPosY, radarSize, radarSize, RGB(150, 150, 150));
        overlay.DrawBox(radarPosX, radarPosY, radarSize, radarSize, RGB(20, 20, 20));
        int radarCenterX = radarPosX + radarSize / 2;
        int radarCenterY = radarPosY + radarSize / 2;

        uintptr_t localPlayer = memory.read<uintptr_t>(memory.base_client_.base + Offsets::LocalPlayer);
        if (localPlayer) {
            int localTeam = memory.read<int>(localPlayer + Offsets::Team);
            Vector3 localOrigin = memory.read<Vector3>(localPlayer + Offsets::Position);

            for (int i = 0; i < 64; i++) {
                uintptr_t entity = memory.read<uintptr_t>(memory.base_client_.base + Offsets::Entitylist + (i * 0x10));
                if (!entity || entity == localPlayer) continue;

                bool isDormant = memory.read<bool>(entity + Offsets::Dormant);
                if (isDormant) continue;

                int team = memory.read<int>(entity + Offsets::Team);
                if (localTeam != -1 && team == localTeam) continue;

                uint8_t lifeState = memory.read<uint8_t>(entity + Offsets::LifeState);
                int health = memory.read<int>(entity + Offsets::Health);
                if (health <= 0 || health > 100 || lifeState != 0) continue;

                if (radar) {
                    Vector3 enemyOrigin = memory.read<Vector3>(entity + Offsets::Position);
                    Vector3 localViewAngles = memory.read<Vector3>(localPlayer + Offsets::ViewAngle);

                    float dx = enemyOrigin.x - localOrigin.x;
                    float dy = enemyOrigin.y - localOrigin.y;

                    float angle = localViewAngles.y * (M_PI / 180.0f); 

                    float rotatedX = dx * cos(angle) - dy * sin(angle);
                    float rotatedY = dx * sin(angle) + dy * cos(angle);

                    rotatedX *= radarScale;
                    rotatedY *= radarScale;

                    int radarX = radarCenterX + static_cast<int>(rotatedX);
                    int radarY = radarCenterY - static_cast<int>(rotatedY); 

                    radarX = max(radarPosX + 2, min(radarPosX + radarSize - 2, radarX));
                    radarY = max(radarPosY + 2, min(radarPosY + radarSize - 2, radarY));

                    COLORREF enemyColor = (team == localTeam) ? RGB(0, 0, 255) : RGB(255, 0, 0);
                    overlay.DrawBox(radarX, radarY, 5, 5, enemyColor);
                }
            }
        }

        overlay.DrawLine(radarCenterX, radarPosY + 2, radarCenterX, radarPosY + radarSize - 2, RGB(20, 20, 20));
        overlay.DrawLine(radarPosX + 2, radarCenterY, radarPosX + radarSize - 2, radarCenterY, RGB(20, 20, 20));

        overlay.EndDraw();
        Sleep(1);
    }

    return 0;
}