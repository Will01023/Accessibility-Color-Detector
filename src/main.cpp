#pragma comment(lib, "dwmapi.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#include <cstdio>
#include <cstdint>

// Read the pixel at screen coordinates (x, y) via GDI
static bool ReadPixel(int x, int y, uint8_t& r, uint8_t& g, uint8_t& b) {
    HDC hdc = GetDC(NULL);
    if (!hdc) return false;
    COLORREF c = GetPixel(hdc, x, y);
    ReleaseDC(NULL, hdc);
    if (c == CLR_INVALID) return false;
    r = GetRValue(c);
    g = GetGValue(c);
    b = GetBValue(c);
    return true;
}

// RGB -> simple color name via HSL
static const char* ClassifyColor(uint8_t r, uint8_t g, uint8_t b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float mx = (rf > gf) ? ((rf > bf) ? rf : bf) : ((gf > bf) ? gf : bf);
    float mn = (rf < gf) ? ((rf < bf) ? rf : bf) : ((gf < bf) ? gf : bf);
    float d  = mx - mn;

    float l = (mx + mn) / 2.0f;
    float s = 0.0f;
    float h = 0.0f;

    if (d > 0.0001f) {
        s = (l > 0.5f) ? d / (2.0f - mx - mn) : d / (mx + mn);
        if (mx == rf)      h = (gf - bf) / d + (gf < bf ? 6.0f : 0.0f);
        else if (mx == gf) h = (bf - rf) / d + 2.0f;
        else               h = (rf - gf) / d + 4.0f;
        h *= 60.0f;
    }

    if (l < 0.12f) return "Black";
    if (l > 0.92f && s < 0.15f) return "White";

    if (s < 0.12f) {
        if (l < 0.35f) return "Dark Gray";
        if (l < 0.65f) return "Gray";
        return "Light Gray";
    }

    if (h < 10.0f  || h >= 350.0f) return "Red";
    if (h < 25.0f)  return "Orange";
    if (h < 48.0f)  return "Yellow";
    if (h < 80.0f)  return "Lime";
    if (h < 160.0f) return "Green";
    if (h < 195.0f) return "Cyan";
    if (h < 255.0f) return "Blue";
    if (h < 290.0f) return "Purple";
    if (h < 330.0f) return "Pink";
    return "Red";
}

// Overlay globals
static HWND g_hwnd = nullptr;
static char g_color_text[64] = "Starting...";
static COLORREF g_swatch_color = RGB(128, 128, 128);
static const int WINDOW_W = 180;
static const int WINDOW_H = 40;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
        FillRect(hdc, &rc, bg);
        DeleteObject(bg);

        // Color swatch
        RECT swatch = { 4, 4, 36, 36 };
        HBRUSH sw = CreateSolidBrush(g_swatch_color);
        FillRect(hdc, &swatch, sw);
        DeleteObject(sw);

        HPEN pen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
        HPEN old_pen = (HPEN)SelectObject(hdc, pen);
        HBRUSH old_br = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, 4, 4, 36, 36);
        SelectObject(hdc, old_br);
        SelectObject(hdc, old_pen);
        DeleteObject(pen);

        // Color name
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(240, 240, 240));
        HFONT font = CreateFontA(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HFONT old_font = (HFONT)SelectObject(hdc, font);

        RECT text_rc = { 42, 0, rc.right - 4, rc.bottom };
        DrawTextA(hdc, g_color_text, -1, &text_rc, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

        SelectObject(hdc, old_font);
        DeleteObject(font);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_NCHITTEST:
        return HTCAPTION;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

static HWND CreateOverlayWindow(HINSTANCE hInst) {
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInst;
    wc.lpszClassName  = "AccessibilityColorDetector";
    wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_APPWINDOW | WS_EX_LAYERED,
        "AccessibilityColorDetector", "Accessibility Color Detector",
        WS_POPUP | WS_VISIBLE | WS_SYSMENU,
        100, 100, WINDOW_W, WINDOW_H,
        nullptr, nullptr, hInst, nullptr
    );

    SetLayeredWindowAttributes(hwnd, 0, 230, LWA_ALPHA);

    HRGN rgn = CreateRoundRectRgn(0, 0, WINDOW_W + 1, WINDOW_H + 1, 10, 10);
    SetWindowRgn(hwnd, rgn, TRUE);

    return hwnd;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hwnd = CreateOverlayWindow(hInstance);
    if (!g_hwnd) return 1;

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    RegisterHotKey(g_hwnd, 1, MOD_CONTROL | MOD_SHIFT, 'Q');

    uint8_t prev_r = 0, prev_g = 0, prev_b = 0;
    const char* prev_name = "";
    DWORD last_update = 0;

    MSG msg;
    while (true) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto done;
            if (msg.message == WM_HOTKEY && msg.wParam == 1) goto done;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        DWORD now = GetTickCount();
        if (now - last_update < 33) {
            Sleep(1);
            continue;
        }
        last_update = now;

        POINT cursor;
        GetCursorPos(&cursor);

        uint8_t r, g, b;
        if (ReadPixel(cursor.x, cursor.y, r, g, b)) {
            const char* name = ClassifyColor(r, g, b);

            if (name != prev_name || r != prev_r || g != prev_g || b != prev_b) {
                prev_r = r; prev_g = g; prev_b = b;
                prev_name = name;
                g_swatch_color = RGB(r, g, b);
                snprintf(g_color_text, sizeof(g_color_text), "%s", name);
                InvalidateRect(g_hwnd, nullptr, FALSE);
            }
        }

        int ox = cursor.x + 20;
        int oy = cursor.y + 25;
        int sw = GetSystemMetrics(SM_CXSCREEN);
        int sh = GetSystemMetrics(SM_CYSCREEN);
        if (ox + WINDOW_W > sw) ox = cursor.x - WINDOW_W - 10;
        if (oy + WINDOW_H > sh) oy = cursor.y - WINDOW_H - 10;

        SetWindowPos(g_hwnd, HWND_TOPMOST, ox, oy, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }

done:
    UnregisterHotKey(g_hwnd, 1);
    DestroyWindow(g_hwnd);
    return 0;
}
