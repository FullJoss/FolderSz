#pragma once
#include <windows.h>

RECT GetNextBounds(int width, int height, BOOL horizontal);
RECT GetFinalBounds(int total, int height, BOOL horizontal);
void SetMargin(int value);
void SetX(int value);
void SetY(int value);