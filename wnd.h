#pragma once
#include "pch.h"
#include "button.h"

typedef struct
{
    Button *btnHead;
    int n_buttons;
} WindowDataInformation;

HWND createWindow(const wchar_t *title, int width, int height, WindowDataInformation *info);
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int launchWindow(HWND hwnd);
void OnPaint();