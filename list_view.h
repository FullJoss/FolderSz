#pragma once
#include "pch.h"
#include "feature.h"

#define IDC_MY_LISTVIEW 5001 // Vous pouvez choisir n'importe quel nombre libre

typedef enum {
    COLUMN_LISTVIEW_NAME,
    COLUMN_LISTVIEW_SIZE
} ColumnListView;

typedef struct {
    HWND hwnd;
    ListFile* data;
    RECT bounds;
} ListFileView;

void DrawListFileView(HDC hdc, ListFileView* lfv);
void UpdateListFileView(ListFileView* lfv);