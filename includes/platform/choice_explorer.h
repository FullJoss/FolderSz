/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "pch.h"

BOOL CreateChoiceExplorer(HWND hwndParent, wchar_t *outPath, DWORD maxLen);
LRESULT CALLBACK AltTabChoixProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);