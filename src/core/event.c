/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#include "pch.h"
#include "event.h"
#include "list_view.h"
#include "dialog.h"
#include "choice_explorer.h"

static wchar_t m_current_path[MAX_PATH] = { 0 };
BOOL g_TriCroissant = FALSE;

void OpenDialogFolder(HWND hwnd, WindowDataInformation* pData)
{
    ListFileView* lfv = pData->lfv;

    wchar_t *chaine = LoadFolderDialog();

    if (chaine == NULL) return;

    ListFile* list = ScanFilePath(chaine, lfv);
    UpdateListFileView(hwnd, lfv, list, TRUE);

    free(chaine);
}

void LaunchChoiceExplorer(HWND hwnd, WindowDataInformation* pData) {
    ListFileView* lfv = pData->lfv;

    if (CreateChoiceExplorer(mainHwnd, m_current_path, MAX_PATH))
    {
        ListFile* list = ScanFilePath(m_current_path, lfv);
        UpdateListFileView(hwnd, lfv, list, TRUE);
    }
    else
        MessageBox_View(L"No Windows Explorer windows are open.");
}

static inline int CompareByDirectory(ElementFile* A, ElementFile* B) {
    BOOL isDirAParent = !!(A->flags & ElementParentDirectory);
    BOOL isDirBParent = !!(B->flags & ElementParentDirectory);
    BOOL isDirA = !!(A->flags & ElementDirectory);
    BOOL isDirB = !!(B->flags & ElementDirectory);

    return (isDirB - isDirA) + (isDirBParent - isDirAParent);
}

int CompareByName(const void* a, const void* b, BOOL isAscending)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    diff = _wcsicmp(elemA->name, elemB->name); // case-insensitive
    return isAscending ? diff : -diff;
}

int CompareBySize(const void* a, const void* b, BOOL isAscending)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    BOOL ABetter = (elemA->size) > (elemB->size);
    BOOL BBetter = (elemA->size) < (elemB->size);
    diff = ABetter - BBetter;
    return isAscending ? diff : -diff;
}