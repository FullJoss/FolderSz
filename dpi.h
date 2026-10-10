#pragma once

#include <windows.h>

// Définitions pour les anciens compilateurs
typedef enum
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS);

void ForcerFenetreNetteParLeCode(void)
{
    // 1. Tenter la méthode Windows 8.1 / 10 (Per-Monitor)
    HMODULE hShcore = GetModuleHandleW(L"shcore.dll");
    if (!hShcore)
        hShcore = LoadLibraryW(L"shcore.dll");

    if (hShcore)
    {
        SetProcessDpiAwarenessProc pSetDpi =
            (SetProcessDpiAwarenessProc)GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (pSetDpi)
        {
            // 2 = PROCESS_PER_MONITOR_DPI_AWARE
            if (SUCCEEDED(pSetDpi(PROCESS_PER_MONITOR_DPI_AWARE)))
            {
                return; // Succès !
            }
        }
    }

    // 2. Solution de secours Windows Vista / 7 / 8 (System Aware)
    // Moins parfaite sur le multi-écran, mais supprime le flou à 100% sur l'écran principal
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