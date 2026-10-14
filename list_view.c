#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include "dialog.h"

BOOL g_TriCroissant = TRUE;
ColumnListView g_sortColumn = COLUMN_LISTVIEW_NAME;

static HWND m_hwnd = NULL;
static ListFile* m_list;

static inline void AttachExplorerImageList(HWND hwnd);
static inline void AddColumn(HWND hwnd, ColumnListView column_index, wchar_t* name, int width);
static inline void ManageColumn(HWND hwnd, int width);
static inline void ChangeSortingMethod(ColumnListView clickedColumn);
static inline void GetText(NMLVDISPINFOW* plvdi, int index);
static inline int CompareByDirectory(ElementFile* A, ElementFile* B);
static inline void SortAt(ColumnListView clickedColumn);


static int CompareByName(const void *a, const void *b)
{
    ElementFile *elemA = (ElementFile *)a;
    ElementFile *elemB = (ElementFile *)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    diff = _wcsicmp(elemA->name, elemB->name); // case-insensitive
    return g_TriCroissant ? diff : -diff;
}

static int CompareBySize(const void *a, const void *b)
{
    ElementFile *elemA = (ElementFile *)a;
    ElementFile *elemB = (ElementFile *)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    BOOL ABetter = (elemA->size) > (elemB->size);
    BOOL BBetter = (elemA->size) < (elemB->size);
    diff = ABetter - BBetter;
    return g_TriCroissant ? diff : -diff;
}

void CreateListView(int x, int y, int width, int height)
{
    HINSTANCE instance = GetModuleHandle(NULL);
    m_hwnd = CreateWindowExW(
        0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA,
        x, y, width, height,
        mainHwnd, (HMENU)IDC_MY_LISTVIEW, instance, NULL);

    ManageColumn(m_hwnd, width);

    ListView_SetExtendedListViewStyle(m_hwnd, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    SetListViewTheme(m_hwnd);
    AttachExplorerImageList(m_hwnd);

    return;
}

static inline BOOL OnColumnClick(LPNMHDR lpnmh)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lpnmh;
    ColumnListView clickedColumn = pnmv->iSubItem;

    ChangeSortingMethod(clickedColumn);
    SortAt(clickedColumn);

    // Refreshes the range of lines
    SendMessage(m_hwnd, LVM_REDRAWITEMS, 0, m_list->count - 1);

    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);

    return TRUE;
}

static void OnGetInfoLines(LPNMHDR lpnmh)
{
    NMLVDISPINFOW* plvdi = (NMLVDISPINFOW*)lpnmh;

    unsigned int index = plvdi->item.iItem;

    if (index >= m_list->count)
        return;

    // Provide the corresponding icon.
    if (plvdi->item.mask & LVIF_IMAGE)
    {
        plvdi->item.iImage = m_list->file[index].indexIcon;
    }

    if (plvdi->item.mask & LVIF_TEXT)
    {
        GetText(plvdi, index);
    }
}

void UpdateListView(ListFile* lf)
{
    qsort(lf->file, lf->count, sizeof(ElementFile), CompareByName);

    SendMessageW(m_hwnd, LVM_SETITEMCOUNT, (WPARAM)lf->count, 0);
    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);

    m_list = lf;
}

BOOL NotifListView(LPARAM lParam)
{
    LPNMHDR lpnmh = (LPNMHDR)lParam;
 
    if (lpnmh->idFrom != IDC_MY_LISTVIEW)
        return FALSE;

    switch (lpnmh->code) {
    case LVN_COLUMNCLICK:
        OnColumnClick(lpnmh);
        break;
    case LVN_GETDISPINFOW:
        OnGetInfoLines(lpnmh);
        break;
    }

    return TRUE;
}


static inline void AttachExplorerImageList(HWND hwnd) {

    SHFILEINFOW sfi = { 0 };

    HIMAGELIST hSystemImageList = (HIMAGELIST)SHGetFileInfoW(
        L"C:\\",
        FILE_ATTRIBUTE_DIRECTORY,
        &sfi,
        sizeof(sfi),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    if (hSystemImageList != NULL)
    {
        ListView_SetImageList(hwnd, hSystemImageList, LVSIL_SMALL);
    }
}

static inline void AddColumn(HWND hwnd, ColumnListView column_index, wchar_t* name, int width) {
    LVCOLUMNW lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = name;
    lvc.cx = width;
    LRESULT hr = SendMessageW(hwnd, LVM_INSERTCOLUMNW, column_index, (LPARAM)&lvc);

    if (FAILED(hr))
        MessageBox_View(L"Failed to add column");
}

static inline void ManageColumn(HWND hwnd, int width) {
    int widthColumnSize = (width - 30) / 3;
    int widthColumnName = (width - 30) - widthColumnSize;

    AddColumn(m_hwnd, COLUMN_LISTVIEW_NAME, L"File name", widthColumnName);
    AddColumn(m_hwnd, COLUMN_LISTVIEW_SIZE, L"Size", widthColumnSize);
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

static inline void GetText(NMLVDISPINFOW* plvdi, int index) {
    switch (plvdi->item.iSubItem) {
    case COLUMN_LISTVIEW_NAME:
        plvdi->item.pszText = m_list->file[index].name;
        return;
    case COLUMN_LISTVIEW_SIZE:
        if (m_list->file[index].flags & ElementSystem)
        {
            plvdi->item.pszText = L"SystemFile";
        }
        else
        {
            ULONGLONG bytesSize = m_list->file[index].size;
            StrFormatByteSizeW((ULONGLONG)bytesSize, plvdi->item.pszText, plvdi->item.cchTextMax);
        }
        return;
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
