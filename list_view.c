#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include "dialog.h"

BOOL g_TriCroissant = TRUE;
ColumnListView g_sortColumn = COLUMN_LISTVIEW_NAME;

static HWND m_hwnd = NULL;
static ListFile* m_list;

static int CompareBySize(const void* a, const void* b);
static int CompareByName(const void* a, const void* b);
static inline void ChangeSortingMethod(ColumnListView clickedColumn);
static inline int CompareByDirectory(ElementFile* A, ElementFile* B);
static inline void SortAt(ColumnListView clickedColumn);

void DrawListFileView(HDC hdc, ListFileView* lfv) {
    FillRect(hdc, &lfv->bounds, g_brushs[IC_BTN_CLICK]);
}

void UpdateListFileView(ListFileView* lfv)
{
    qsort(lfv->data->file, lfv->data->count, sizeof(ElementFile), CompareByName);
    InvalidateRect(lfv->hwnd, &lfv->bounds, FALSE);
}


static int CompareByName(const void* a, const void* b)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    diff = _wcsicmp(elemA->name, elemB->name); // case-insensitive
    return g_TriCroissant ? diff : -diff;
}

static int CompareBySize(const void* a, const void* b)
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

static inline void ChangeSortingMethod(ColumnListView clickedColumn) {
    if (g_sortColumn == clickedColumn)
    {
        g_TriCroissant = !g_TriCroissant;
    }
    else
    {
        g_sortColumn = clickedColumn;
        g_TriCroissant = TRUE;
    }
}

static inline int CompareByDirectory(ElementFile* A, ElementFile* B) {
    BOOL isDirA = !!(A->flags & ElementDirectory);
    BOOL isDirB = !!(B->flags & ElementDirectory);

    return isDirB - isDirA;
}

static inline void SortAt(ColumnListView clickedColumn) {
    switch (clickedColumn) {
    case COLUMN_LISTVIEW_NAME:
        qsort(m_list->file, m_list->count, sizeof(ElementFile), CompareByName);
        break;
    case COLUMN_LISTVIEW_SIZE:
        qsort(m_list->file, m_list->count, sizeof(ElementFile), CompareBySize);
        break;
    }
}
