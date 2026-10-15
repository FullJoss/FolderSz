#pragma once
#include "type.h"

#define MAX_LISTVIEW_ELEMENT 5

extern wchar_t *current_path;

wchar_t *LoadFolderDialog();

ListFile* ScanFilePath(const wchar_t *dossierPath);

// Call at cache.c
void RefreshView();

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