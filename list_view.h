#pragma once
#include "pch.h"
#include "type.h"
#include "feature.h"

void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint);
void HandleListViewLButtonUp(HWND hWnd, ListFileView* listView);
void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewMouseWheel(HWND hWnd, int wheelDelta, LPARAM lParam, ListFileView* listView);
void UpdateListFileView(ListFileView* lfv);
void FreeListFileView(ListFileView* lfv);

static inline int RectW(const RECT* r) { return r->right - r->left; }
static inline int RectHeight(const RECT* r) { return r->bottom - r->top; }