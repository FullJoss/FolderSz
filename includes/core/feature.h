/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "type.h"

#define MAX_LISTVIEW_ELEMENT 5

extern wchar_t *current_path;

wchar_t *LoadFolderDialog();

ListFile* ScanFilePath(const wchar_t *dossierPath, ListFileView* lfv);

void OnDoubleClickRow(HWND hwnd, ListFileView* lfv);

// Call at cache.c
void RefreshView(ListFileView* lfv);

static inline void AddFlagsElement(ElementFile *element, ElementFlags flag)
{
    element->flags |= flag;
}
static inline void RemoveFlagsElement(ElementFile *element, ElementFlags flag)
{
    element->flags &= ~flag;
}
static inline BOOL CompareFlagsElement(ElementFile *A, ElementFile *B, ElementFlags flag)
{
    return ((A->flags ^ B->flags) & flag);
}