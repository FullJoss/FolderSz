#pragma once

#include <windows.h>

// Definitions for older compilers
typedef enum
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS);

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
            (BOOL(WINAPI *)(void))GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetDpiOld)
        {
            pSetDpiOld();
        }
    }
}