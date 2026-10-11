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
    ForcerFenetreNetteParLeCode();

    init_theme();

    int window_width = 500;

    Button buttons[NB_BUTTON];

    int accx = 10;
    int accy = 10;
    int accwidth = 150;
    int accheight = 38;

    buttons[0].bounds = (RECT){accx, accy, accx + accwidth, accy + accheight};
    buttons[0].state = PBS_NORMAL;
    buttons[0].text = L"Ovrir un dossier";
    buttons[0].on_click = OpenDialogFolder;

    accx += accwidth + 10;
    accwidth = window_width - accx - 40;

    buttons[1].bounds = (RECT){accx, accy, accx + accwidth, accy + accheight};
    buttons[1].state = PBS_NORMAL;
    buttons[1].text = L"Resynchroniser avec Explorer";
    buttons[1].on_click = LaunchChoiceExplorer;

    WindowDataInformation info;
    info.btnHead = buttons;
    info.n_buttons = NB_BUTTON;

    mainHwnd = createWindow(L"JossFileSize", window_width, 600, &info);
    ActiverDarkModeUltra(mainHwnd);

    init_list_view(10, 60, window_width - 30, 500);
    LaunchChoiceExplorer();
    launchWindow(mainHwnd);

    close_theme();

    return 0;
}