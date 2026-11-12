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
volatile BOOL thrd_scanner_isRun = FALSE;

void OpenDialogFolder(HWND hwnd, WindowDataInformation* pData)
{
    static ScanArg scanData;
    
    wchar_t* chaine = LoadFolderDialog();
    if (chaine == NULL) return;

    SetWindowTextW(hwnd, chaine);

    ListFile* lf = ScanFilePath(chaine);
    UpdateListFileView(hwnd, pData->lfv, lf, TRUE);

    scanData.lf = lf;
    scanData.path = chaine;

    thrd_scanner_isRun = TRUE;
    unsigned threadId;
    HANDLE hThread = (HANDLE)_beginthreadex(
        NULL,        
        0,              
        &AsyncScanFolders,    
        &scanData,   
        0,              
        &threadId     
    );

    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
    else {
        thrd_scanner_isRun = FALSE;
    }
}

void LaunchChoiceExplorer(HWND hwnd, WindowDataInformation* pData) {
    ListFileView* lfv = pData->lfv;

    if (CreateChoiceExplorer(hwnd, m_current_path, MAX_PATH))
    {
        ListFile* list = ScanFilePath(m_current_path);
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