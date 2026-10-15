#pragma once
#include "pch.h"
#include "event.h"


BOOL ButtonHitTest(WPARAM wParam, int x, int y, Button *button);
BOOL ButtonClickTest(WPARAM wParam, int x, int y, Button* button, WindowDataInformation* pData);
void ButtonDraw(HDC hdc, Button *button);

Button CreateButton(const wchar_t* name, Callback event);