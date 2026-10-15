#include "pch.h"
#include "event.h"
#include "list_view.h"
#include "dialog.h"
#include "choice_explorer.h"

static wchar_t m_current_path[MAX_PATH] = { 0 };
BOOL g_TriCroissant = TRUE;

void OpenDialogFolder(HWND hwnd, WindowDataInformation* pData)
{
    ListFileView* lfv = pData->lfv;

    wchar_t *chaine = LoadFolderDialog();

    if (chaine == NULL) return;

    ListFile* list = ScanFilePath(chaine);
    UpdateListFileView(hwnd, lfv, list);

    free(chaine);
}

void LaunchChoiceExplorer(HWND hwnd, WindowDataInformation* pData) {
    ListFileView* lfv = pData->lfv;

    if (CreateChoiceExplorer(mainHwnd, m_current_path, MAX_PATH))
    {
        ListFile* list = ScanFilePath(m_current_path);
        UpdateListFileView(hwnd, lfv, list);
    }
    else
        MessageBox_View(L"No Windows Explorer windows are open.");
}

static inline int CompareByDirectory(ElementFile* A, ElementFile* B) {
    BOOL isDirA = !!(A->flags & ElementDirectory);
    BOOL isDirB = !!(B->flags & ElementDirectory);

    return isDirB - isDirA;
}

int CompareByName(const void* a, const void* b)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    diff = _wcsicmp(elemA->name, elemB->name); // case-insensitive
    return g_TriCroissant ? diff : -diff;
}

int CompareBySize(const void* a, const void* b)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    BOOL ABetter = (elemA->size) > (elemB->size);
    BOOL BBetter = (elemA->size) < (elemB->size);
    diff = ABetter - BBetter;
    return g_TriCroissant ? diff : -diff;
}