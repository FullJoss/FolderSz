#include "dpi.h"

void ForceSharpWindowViaCode(void)
{
    // Per-Monitor Method
    HMODULE hShcore = GetModuleHandleW(L"shcore.dll");
    if (!hShcore)
        hShcore = LoadLibraryW(L"shcore.dll");

    if (hShcore)
    {
        SetProcessDpiAwarenessProc pSetDpi =
            (SetProcessDpiAwarenessProc)GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (pSetDpi)
        {
            if (SUCCEEDED(pSetDpi(PROCESS_PER_MONITOR_DPI_AWARE)))
            {
                return;
            }
        }
    }

    // SystemAware Method
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32)
    {
        BOOL(WINAPI * pSetDpiOld)(void) =
            (BOOL(WINAPI*)(void))GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetDpiOld)
        {
            pSetDpiOld();
        }
    }


}

int GetFontHeightDPI(HWND hwnd, HFONT font) {
    HDC hdcLocal = GetDC(hwnd);
    HFONT hOldFont = (HFONT)SelectObject(hdcLocal, font);

    TEXTMETRICW tm;
    GetTextMetricsW(hdcLocal, &tm);

    int textRenderHeight = tm.tmHeight;

    SelectObject(hdcLocal, hOldFont);
    ReleaseDC(hwnd, hdcLocal);

    return textRenderHeight;
}

int GetDpiForWindowSafe(HWND hwnd) {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef UINT(WINAPI* PFN_GetDpiForWindow)(HWND);
        PFN_GetDpiForWindow pGetDpi = (PFN_GetDpiForWindow)GetProcAddress(hUser32, "GetDpiForWindow");
        if (pGetDpi) {
            return pGetDpi(hwnd);
        }
    }
    // Fallback 
    return 96;
}