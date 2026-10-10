#pragma once
#include "pch.h"

#define IDC_MY_LISTVIEW 5001 // Vous pouvez choisir n'importe quel nombre libre

extern HWND list_view_hwnd;

void init_list_view(int, int, int, int);

int List_View_Notification(LPARAM lParam);
void List_View_MAJ();