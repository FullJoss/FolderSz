#include "pch.h"

#include "dpi.h"
#include "theme.h"
#include "wnd.h"
#include "list_view.h"
#include "choice_explorer.h"
#include "dialog.h"
#include "feature.h"
#include "event.h"
#include "layout.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "dwmapi.lib")

#define NB_BUTTON 2
#define NB_LIST_COLUMN 2
#define WIN_WIDTH 400
#define WIN_HEIGHT 500

HWND mainHwnd = NULL;
int DPI_VALUE = 96;

int main()
{
    ForceSharpWindowViaCode();

    InitTheme();


    Button buttons[NB_BUTTON];
    ListFileView list = { 0 };
    WindowDataInformation info = { buttons, &list, NB_BUTTON, WIN_WIDTH, WIN_HEIGHT };
    mainHwnd = CreateMainWindow(L"JossFileSize", &info);
    g_fontHeight = GetFontHeightDPI(mainHwnd, g_font);

    DPI_VALUE = GetDpiForWindowSafe(mainHwnd);

    buttons[0] = CreateButton(L"Open foler", OpenDialogFolder);
    buttons[1] = CreateButton(L"Resynchronize with Explorer", LaunchChoiceExplorer);
  


    InitLayout(DPI_CONST(WIN_WIDTH), DPI_CONST(WIN_HEIGHT));

    buttons[0].bounds = g_rects[RECT_LAYOUT_BUTTON_OPEN_FOLDER];
    buttons[1].bounds = g_rects[RECT_LAYOUT_BUTTON_SYNC_EXPLR];
    list.bounds = g_rects[RECT_LAYOUT_LIST_VIEW];



    ColumnListView columns[NB_LIST_COLUMN] = { 0 };
    columns[0].name = L"Name";
    columns[0].width = GetWFillOfThird(RECT_LAYOUT_LIST_VIEW);
    columns[0].FuncComparison = CompareByName;

    columns[1].name = L"Size";
    columns[1].width = GetWThird(RECT_LAYOUT_LIST_VIEW);
    columns[1].FuncComparison = CompareBySize;
    

    list.columns = columns;
    list.n_columns = NB_LIST_COLUMN;
    list.indexColSort = 1;
    list.row_size = sizeof(ElementFile);


    LaunchChoiceExplorer(mainHwnd, &info);








    int code = LaunchWindow(mainHwnd);

    FreeListFileView(&list);

    FreeTheme();

    return code;
}