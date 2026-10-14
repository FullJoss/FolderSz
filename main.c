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
#define WIN_WIDTH 500
#define WIN_HEIGHT 600

HWND mainHwnd = NULL;

int main()
{
    ForceSharpWindowViaCode();

    InitTheme();

    Button buttons[NB_BUTTON];
    WindowDataInformation info = { buttons, NB_BUTTON, NULL };
    buttons[0] = CreateButton(L"Open a file", OpenDialogFolder);
    buttons[1] = CreateButton(L"Resynchronize with Explorer", LaunchChoiceExplorer);
    mainHwnd = CreateMainWindow(L"JossFileSize", WIN_WIDTH, WIN_HEIGHT, &info);

    SetY(10);
    SetMargin(10);
    buttons[0].bounds = GetNextBounds(150, 38, TRUE);
    buttons[1].bounds = GetFinalBounds(WIN_WIDTH, 38, TRUE);
  
    int y = 38 + 10 + 10;
    ListFileView lfv = { 0 };
    lfv.bounds = (RECT){ 10, y, WIN_WIDTH - 10, WIN_HEIGHT - 10 };
    lfv.clipRgn = CreateRectRgnIndirect(&lfv.bounds);
    lfv.hwnd = mainHwnd;
    lfv.data = LaunchChoiceExplorer();
    info.lfv = &lfv;

    UpdateListFileView(&lfv);

    int code = LaunchWindow(mainHwnd);

    DeleteListFileView(&lfv);

    FreeTheme();

    return code;
}