#pragma once
#include "pch.h"
#include "event.h"

typedef struct
{
    RECT bounds;
    int state;
    const wchar_t *text;
    Callback on_click;
} Button;

BOOL ButtonHitTest(WPARAM wParam, int x, int y, Button *button);
BOOL ButtonClickTest(WPARAM wParam, int x, int y, Button *button);

void ButtonDraw(HDC hdc, Button *button);

static inline Button CreateButton(const wchar_t* name, Callback event);