#pragma once
#include "pch.h"
#include "feature.h"

typedef struct {
    const wchar_t* name;
    unsigned int width;
    BOOL isSorted;
    int (*FuncComparison) (const void*, const void*);
} ColumnListView;

typedef struct {
    RECT bounds;
    HWND hwnd;
    ListFile* data;
    int scrollPos;
    int dragYOffset;
    int indexHover;
    BOOL isDraggingScroll; 

    RECT itemsArea;
    HRGN clipRgn;
    RECT sbTrack;
    int sbThumbHeight;

    ColumnListView* columns;
    int n_columns;
} ListFileView;

void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint);
void HandleListViewLButtonUp(HWND hWnd, ListFileView* listView);
void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewMouseWheel(HWND hWnd, int wheelDelta, LPARAM lParam, ListFileView* listView);
void UpdateListFileView(ListFileView* lfv);
void FreeListFileView(ListFileView* lfv);

static inline int RectW(const RECT* r) { return r->right - r->left; }
static inline int RectHeight(const RECT* r) { return r->bottom - r->top; }