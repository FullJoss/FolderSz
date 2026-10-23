/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "pch.h"
#include "type.h"
#include "feature.h"

void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint);
void HandleListViewLButtonUp(HWND hWnd, ListFileView* listView);
void HandleListViewLButtonDoubleClick(HWND hWnd, int x, int y, ListFileView* lfv);
void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewMouseWheel(HWND hWnd, int wheelDelta, LPARAM lParam, ListFileView* listView);
void HandleListViewKeyDown(HWND hwnd, wchar_t key, ListFileView* lfv);
void UpdateListFileView(HWND hwnd, ListFileView* lfv, ListFile* newlist, BOOL sort);
void FreeListFileView(ListFileView* lfv);
void UpdateDPIListView();

static inline int RectW(const RECT* r) { return r->right - r->left; }
static inline int RectHeight(const RECT* r) { return r->bottom - r->top; }