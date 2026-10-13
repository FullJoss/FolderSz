#pragma once

#include "pch.h"

typedef enum {
    IC_WINDOW_COLOR,
    IC_BTN_BACKGROUND,
    IC_BTN_HOVER,
    IC_BTN_CLICK,
    IC_BTN_BORDER,
    IC_BTN_HOVER_BORDER,
    IC_TEXT,
    IC_COUNT
} ColorIndex;

extern COLORREF g_colors[IC_COUNT];
extern HBRUSH g_brushs[IC_COUNT];
extern HFONT g_font;
extern HPEN CONST_PEN_SELECTION;

void InitTheme();
void FreeTheme();
void SetListViewTheme(HWND);
void SetTitleWindowTheme(HWND hWnd);