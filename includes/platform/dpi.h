/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once

#include <windows.h>

// Definitions for older compilers
typedef enum
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

extern int DPI_VALUE;
#define DPI_CONST(value) (((DPI_VALUE) * (value)) / 96)

typedef HRESULT(WINAPI *SetProcessDpiAwarenessProc)(PROCESS_DPI_AWARENESS);

void ForceSharpWindowViaCode(void);
int GetDpiForWindowSafe(HWND hwnd);
int GetFontHeightDPI(HWND hwnd, HFONT font);