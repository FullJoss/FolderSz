/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "type.h"

#define MAX_LISTVIEW_ELEMENT 5

typedef struct {
    wchar_t* path;
    ListFile* lf;
} ScanArg;

extern wchar_t *current_path;

wchar_t *LoadFolderDialog();

void AsyncScanFolders(void* arg);

ListFile* ScanFilePath(const wchar_t *dossierPath);

void OnDoubleClickRow(HWND hwnd, ListFileView* lfv);

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