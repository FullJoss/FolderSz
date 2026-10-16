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

