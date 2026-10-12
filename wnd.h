#pragma once
#include "pch.h"
#include "button.h"

typedef struct
{
    Button *btnsHead;
    int n_buttons;
} WindowDataInformation;

HWND CreateMainWindow(const wchar_t *title, int width, int height, WindowDataInformation *info);
int LaunchWindow(HWND hwnd);