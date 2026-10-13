#include "pch.h"

#include "dpi.h"
#include "theme.h"
#include "wnd.h"
#include "list_view.h"
#include "choice_explorer.h"
#include "dialog.h"
#include "feature.h"
#include "event.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

#define NB_BUTTON 2

HWND mainHwnd = NULL;

int main()
{
    ForceSharpWindowViaCode();

    InitTheme();

    Button buttons[NB_BUTTON];
    buttons[0] = CreateButton(L"Open a file", OpenDialogFolder);
    buttons[1] = CreateButton(L"Resynchronize with Explorer", LaunchChoiceExplorer);

    int window_width = 500;
    int window_height = 600;

    int accx = 10;
    int accy = 10;
    int accwidth = 150;
    int accheight = 38;

    buttons[0].bounds = (RECT){accx, accy, accx + accwidth, accy + accheight};

    accx += accwidth + 10;
    accwidth = window_width - accx - 40;

    buttons[1].bounds = (RECT){accx, accy, accx + accwidth, accy + accheight};

    WindowDataInformation info;
    info.btnsHead = buttons;
    info.n_buttons = NB_BUTTON;

    mainHwnd = CreateMainWindow(L"JossFileSize", window_width, window_height, &info);
    SetTitleWindowTheme(mainHwnd);

    CreateListView(10, 60, window_width - 30, 500);

    LaunchChoiceExplorer();

    int result_code = LaunchWindow(mainHwnd);

    FreeTheme();

    return result_code;
}