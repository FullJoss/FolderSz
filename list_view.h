#pragma once
#include "pch.h"

#define IDC_MY_LISTVIEW 5001 // Vous pouvez choisir n'importe quel nombre libre

typedef enum {
    COLUMN_LISTVIEW_NOM,
    COLUMN_LISTVIEW_SIZE
} ColumnListView;

extern HWND list_view_hwnd;

void init_list_view(int, int, int, int);

int NotifListView(LPARAM lParam);
void UpdateListView();