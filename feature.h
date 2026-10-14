#pragma once

typedef enum
{
    ElementInvalid = 0,        // 00000000 (Aucun état)
    ElementDirectory = 1 << 0, // 00000001 (1)
    ElementSystem = 1 << 1,    // 00000010 (2)
    ElementPartition = 1 << 2, // 00000100 (4)
    ElementCache = 1 << 3      // 00001000 (8)
} ElementFlags;

typedef struct
{
    wchar_t name[MAX_PATH];
    ULONGLONG size;
    ElementFlags flags;
    int indexIcon;
} ElementFile;

typedef struct {
    ElementFile* file;
    unsigned int count;
    unsigned int capacity;
} ListFile;

#define MAX_LISTVIEW_ELEMENT

extern wchar_t *current_path;

wchar_t *LoadFolderDialog();

const ListFile* ScanFilePath(const wchar_t *dossierPath);

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