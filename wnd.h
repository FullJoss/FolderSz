/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "pch.h"
#include "type.h"
#include "button.h"
#include "list_view.h"

HWND CreateMainWindow(const wchar_t *title, WindowDataInformation *info);
int LaunchWindow(HWND hwnd);