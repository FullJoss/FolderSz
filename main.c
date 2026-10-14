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
    WindowDataInformation info = { buttons, NB_BUTTON };
    buttons[0] = CreateButton(L"Open a file", OpenDialogFolder);
    buttons[1] = CreateButton(L"Resynchronize with Explorer", LaunchChoiceExplorer);
    mainHwnd = CreateMainWindow(L"JossFileSize", WIN_WIDTH, WIN_HEIGHT, &info);

    SetY(10);
    SetMargin(10);
    buttons[0].bounds = GetNextBounds(150, 38, TRUE);
    buttons[1].bounds = GetFinalBounds(WIN_WIDTH, 38, TRUE);
  
    CreateListView(10, 60, WIN_WIDTH - 10, 600 - 38 - 10 - 10);

    LaunchChoiceExplorer();

    int code = LaunchWindow(mainHwnd);

    FreeTheme();

    return code;
}