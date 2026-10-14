#pragma once

#ifdef _DEBUG
#define DEBUG_PRINT(format, ...) \
    do { \
        wchar_t debug_buffer[256]; \
        swprintf_s(debug_buffer, 256, L"[DEBUG] " format L"\n", ##__VA_ARGS__); \
        OutputDebugStringW(debug_buffer); \
    } while(0)
#else
// En mode Release, la macro ne génère STRICTEMENT AUCUN CODE (0% impact CPU)
#define DEBUG_PRINT(format, ...) ((void)0)
#endif

void MessageBox_View(const wchar_t *text);
int MessageBox_Confirm(const wchar_t *text);