#pragma once
#include <windows.h>
#include <string>

class Overlay {
private:
    HWND hwnd;
    HDC hdcMem = nullptr;
    HBITMAP hBitmap = nullptr;
    int lastWidth = 0;
    int lastHeight = 0;

public:
    Overlay(HWND hwnd);
    ~Overlay();

    void BeginDraw();
    void EndDraw();
    void DrawLine(int x1, int y1, int x2, int y2, COLORREF color, int strokeWidth = 1);
    void DrawBox(int x, int y, int width, int height, COLORREF color, int strokeWidth = 1);
    void DrawFilledBox(int x, int y, int width, int height, COLORREF fillColor);
    void DrawFilledCircle(int x, int y, int radius, COLORREF fillColor);
};