#include "pch.h"
#include "event.h"
#include "list_view.h"
#include "dialog.h"
#include "choice_explorer.h"

static wchar_t m_current_path[MAX_PATH] = { 0 };
BOOL g_TriCroissant = TRUE;

const ListFile* OpenDialogFolder(void)
{
    wchar_t *chaine = LoadFolderDialog();

    if (chaine == NULL) return NULL;

    const ListFile* lf = ScanFilePath(chaine);

    free(chaine);

    return lf;
}

const ListFile* LaunchChoiceExplorer(void) {
    if (CreateChoiceExplorer(mainHwnd, m_current_path, MAX_PATH))
    {
        return ScanFilePath(m_current_path);
    }
    else
        MessageBox_View(L"No Windows Explorer windows are open.");

    return NULL;
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