#pragma once
#include "pch.h"
#include "feature.h"

#define IDC_MY_LISTVIEW 5001 // Vous pouvez choisir n'importe quel nombre libre

typedef enum {
    COLUMN_LISTVIEW_NAME,
    COLUMN_LISTVIEW_SIZE
} ColumnListView;

void CreateListView(int x, int y, int width, int height);

int NotifListView(LPARAM lParam);
void UpdateListView(ListFile* lf);