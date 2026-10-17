/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */

#pragma once
#include "pch.h"
#include "event.h"


BOOL ButtonHitTest(HWND hwnd,WPARAM wParam, int x, int y, Button *button);
BOOL ButtonClickTest(HWND hwnd, WPARAM wParam, int x, int y, Button* button, WindowDataInformation* pData);
void ButtonDraw(HDC hdc, Button *button);

Button CreateButton(const wchar_t* name, Callback event);