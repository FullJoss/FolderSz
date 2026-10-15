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
#define WIN_WIDTH 500
#define WIN_HEIGHT 600

HWND mainHwnd = NULL;

int main()
{
    ForceSharpWindowViaCode();

    InitTheme();


    Button buttons[NB_BUTTON];
    ListFileView list = { 0 };
    WindowDataInformation info = { buttons, NB_BUTTON, &list };
    mainHwnd = CreateMainWindow(L"JossFileSize", WIN_WIDTH, WIN_HEIGHT, &info);


    SetY(10);
    SetMargin(10);
    buttons[0] = CreateButton(L"Open a file", OpenDialogFolder);
    buttons[0].bounds = GetNextBounds(150, 38, TRUE);

    buttons[1] = CreateButton(L"Resynchronize with Explorer", LaunchChoiceExplorer);
    buttons[1].bounds = GetFinalBounds(WIN_WIDTH, 38, TRUE);
  



    int y = 38 + 10 + 10;
    list.bounds = (RECT){ 10, y, WIN_WIDTH - 10, WIN_HEIGHT - 10 };
    list.row_size = sizeof(ElementFile);
    int list_width = list.bounds.right - list.bounds.left;
    int list_width_third = list_width / 3;
    int list_width_fill = list_width - list_width_third;

    ColumnListView columns[NB_LIST_COLUMN] = { 0 };
    columns[0].name = L"Name";
    columns[0].width = list_width_fill;
    columns[0].FuncComparison = CompareByName;

    columns[1].name = L"Size";
    columns[1].width = list_width_third;
    columns[1].FuncComparison = CompareBySize;
    columns[1].isSorted = TRUE;

    list.columns = columns;
    list.n_columns = NB_LIST_COLUMN;



    LaunchChoiceExplorer(mainHwnd, &info);








    int code = LaunchWindow(mainHwnd);

    FreeListFileView(&list);

    FreeTheme();

    return code;
}