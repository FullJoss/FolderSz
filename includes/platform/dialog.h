/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once

#ifdef _DEBUG
#define DEBUG_PRINT(format, ...) \
    do { \
        wchar_t debug_buffer[256]; \
        swprintf_s(debug_buffer, 256, L"[DEBUG] " format L"\n", ##__VA_ARGS__); \
        OutputDebugStringW(debug_buffer); \
    } while(0)
#else
#define DEBUG_PRINT(format, ...) ((void)0)
#endif

void MessageBox_Error(const wchar_t* text, const wchar_t* caption);
void MessageBox_View(const wchar_t *text);
BOOL MessageBox_Confirm(const wchar_t* text, BOOL preferYes);