#pragma once
#include "pch.h"
#include "button.h"
#include "list_view.h"

typedef struct
{
    Button *buttons;
    int n_buttons;
    ListFileView* lfv;
} WindowDataInformation;

HWND CreateMainWindow(const wchar_t *title, int width, int height, WindowDataInformation *info);
int LaunchWindow(HWND hwnd);