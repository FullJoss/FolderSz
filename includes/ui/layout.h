/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include <windows.h>

typedef enum {
	RECT_LAYOUT_BUTTON_OPEN_FOLDER,
	RECT_LAYOUT_BUTTON_SYNC_EXPLR,
	RECT_LAYOUT_LIST_VIEW,
	RECT_LAYOUT_COUNT
} RectLayout;

extern RECT g_rects[RECT_LAYOUT_COUNT];

void InitLayout(int windowWidth, int windowHeight);
int GetWThird(RectLayout rectTarget);
int GetWFillOfThird(RectLayout rectTarget);

