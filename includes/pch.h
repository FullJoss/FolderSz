/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <shobjidl.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <windowsx.h>
#include <wchar.h>
#include <commctrl.h>
#include <shlwapi.h> // <-- Indispensable pour StrFormatByteSizeW
#include <shlobj.h>
#include <exdisp.h>
#include <dwmapi.h>

extern HWND mainHwnd;