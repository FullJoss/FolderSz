/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include <windows.h>
#include <exdisp.h>
#include <shlobj.h>
#include "type.h"
#include "choice_explorer.h"

int Backend_GetOpenExplorers(ExplorerInfo* outList, int maxCapacity);
wchar_t* Backend_ExctractFilePath(IShellItem* pItem);
IShellItem* Backend_DeployDialogSelector(IFileOpenDialog* pDlg);

BOOL GrowCapacity(void** ppArray, int* actualCapacity, size_t elementSize);