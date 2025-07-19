#include "Overlay.h"

Overlay::Overlay(HWND hwnd) : hwnd(hwnd) {
    HDC hdc = GetDC(hwnd);
    hdcMem = CreateCompatibleDC(hdc);
    ReleaseDC(hwnd, hdc);
}

Overlay::~Overlay() {
    if (hBitmap) DeleteObject(hBitmap);
    if (hdcMem) DeleteDC(hdcMem);
}

void Overlay::BeginDraw() {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (width != lastWidth || height != lastHeight) {
        if (hBitmap) DeleteObject(hBitmap);
        HDC hdc = GetDC(hwnd);
        hBitmap = CreateCompatibleBitmap(hdc, width, height);
        ReleaseDC(hwnd, hdc);
        SelectObject(hdcMem, hBitmap);
        lastWidth = width;
        lastHeight = height;
    }

    HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);
}

void Overlay::EndDraw() {
    RECT rect;
    GetClientRect(hwnd, &rect);
    HDC hdc = GetDC(hwnd);
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void Overlay::DrawLine(int x1, int y1, int x2, int y2, COLORREF color, int strokeWidth) {
    HPEN hPen = CreatePen(PS_SOLID, strokeWidth, color);
    HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);
    MoveToEx(hdcMem, x1, y1, NULL);
    LineTo(hdcMem, x2, y2);
    SelectObject(hdcMem, oldPen);
    DeleteObject(hPen);
}

void Overlay::DrawBox(int x, int y, int width, int height, COLORREF color, int strokeWidth) {
    HPEN hPen = CreatePen(PS_SOLID, strokeWidth, color);
    HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);
    HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
    Rectangle(hdcMem, x, y, x + width, y + height);
    SelectObject(hdcMem, oldBrush);
    SelectObject(hdcMem, oldPen);
    DeleteObject(hPen);
}

void Overlay::DrawFilledBox(int x, int y, int width, int height, COLORREF fillColor) {
    HBRUSH hBrush = CreateSolidBrush(fillColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
    Rectangle(hdcMem, x, y, x + width, y + height);
    SelectObject(hdcMem, oldBrush);
    DeleteObject(hBrush);
}

void Overlay::DrawFilledCircle(int x, int y, int radius, COLORREF fillColor) {
    HBRUSH hBrush = CreateSolidBrush(fillColor);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

    HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);

    Ellipse(hdcMem, x - radius, y - radius, x + radius, y + radius);

    SelectObject(hdcMem, oldBrush);
    SelectObject(hdcMem, oldPen);
    DeleteObject(hBrush);
}

COLORREF HSVtoRGB(float hue, float saturation, float value)
{
    float c = value * saturation;
    float x = c * (1 - fabsf(fmodf(hue / 60.0f, 2) - 1));
    float m = value - c;

    float r, g, b;

    if (hue < 60) { r = c; g = x; b = 0; }
    else if (hue < 120) { r = x; g = c; b = 0; }
    else if (hue < 180) { r = 0; g = c; b = x; }
    else if (hue < 240) { r = 0; g = x; b = c; }
    else if (hue < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    BYTE R = (BYTE)((r + m) * 255);
    BYTE G = (BYTE)((g + m) * 255);
    BYTE B = (BYTE)((b + m) * 255);

    return RGB(R, G, B);
}

