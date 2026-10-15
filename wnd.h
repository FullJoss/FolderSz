#pragma once
#include "pch.h"
#include "type.h"
#include "button.h"
#include "list_view.h"

HWND CreateMainWindow(const wchar_t *title, int width, int height, WindowDataInformation *info);
int LaunchWindow(HWND hwnd);