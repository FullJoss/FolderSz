#pragma once
#include "pch.h"

typedef struct
{

    RECT bounds;
    int state;
    const wchar_t *text;
    void (*on_click)(void);

} Button;

BOOL ButtonHitTest(WPARAM wParam, int x, int y, Button *button);
BOOL ButtonClickTest(WPARAM wParam, int x, int y, Button *button);
void ButtonDraw(HDC hdc, Button *button);