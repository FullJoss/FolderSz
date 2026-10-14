#pragma once
#include "pch.h"
#include "feature.h"

#define IDC_MY_LISTVIEW 5001 // Vous pouvez choisir n'importe quel nombre libre

typedef enum {
    COLUMN_LISTVIEW_NAME,
    COLUMN_LISTVIEW_SIZE
} ColumnListView;

typedef struct {
    RECT bounds;
    HWND hwnd;
    ListFile* data;
    float scrollPos;
    int dragYOffset;
    int indexHover;
    BOOL isDraggingScroll; 

    HRGN clipRgn;
} ListFileView;

void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint);
RECT GetScrollThumbRect(ListFileView* listView);
void HandleListViewLButtonUp(HWND hWnd, ListFileView* listView);
void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* listView);
void HandleListViewMouseWheel(HWND hWnd, int wheelDelta, LPARAM lParam, ListFileView* listView);
void UpdateListFileView(ListFileView* lfv);
void DeleteListFileView(ListFileView* lfv);