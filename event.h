#pragma once
#include "feature.h"
#include "wnd.h"
#include "type.h"


void LaunchChoiceExplorer(HWND, WindowDataInformation*);
void OpenDialogFolder(HWND, WindowDataInformation*);

int CompareByName(const void* a, const void* b);
int CompareBySize(const void* a, const void* b);