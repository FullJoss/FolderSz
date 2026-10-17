/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "feature.h"
#include "wnd.h"
#include "type.h"


void LaunchChoiceExplorer(HWND, WindowDataInformation*);
void OpenDialogFolder(HWND, WindowDataInformation*);

int CompareByName(const void* a, const void* b, BOOL isAscending);
int CompareBySize(const void* a, const void* b, BOOL isAscending);