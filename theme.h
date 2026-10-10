#pragma once

#include "pch.h"

extern COLORREF btn_colors[5];
extern HBRUSH btn_brushs[4];
extern HBRUSH window_brush;
extern HFONT g_font;

void init_theme();
void close_theme();
void set_listview_theme(HWND);
void ActiverDarkModeUltra(HWND hWnd);

static inline HBRUSH get_btn_brush(int state)
{
    return btn_brushs[state - 1];
}
static inline HBRUSH get_btn_brush_border()
{
    return btn_brushs[3];
}

static inline COLORREF get_btn_color_text()
{
    return btn_colors[4];
}