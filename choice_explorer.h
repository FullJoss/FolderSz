#pragma once
#include "pch.h"

BOOL CreateChoiceExplorer(HWND hwndParent, wchar_t *outPath, DWORD maxLen);
LRESULT CALLBACK AltTabChoixProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);