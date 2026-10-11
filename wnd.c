#include "wnd.h"
#include "theme.h"
#include "feature.h"
#include "dialog.h"
#include "Button.h"
#include "theme.h"
#include "list_view.h"
#include "cache.h"

HWND createWindow(const wchar_t *title, int width, int height, WindowDataInformation *info)
{
    HINSTANCE hinstance = GetModuleHandle(NULL);

    DWORD styleFixe = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = L"WINDOW_SIMPLE";
    wc.hbrBackground = window_brush;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(hinstance, MAKEINTRESOURCEW(1));

    RegisterClassW(&wc);

    // 3. Création de la fenêtre avec CreateWindowEx
    return CreateWindowExW(
        0,                            // 1. dwExStyle (Styles étendus)
        L"WINDOW_SIMPLE",             // 2. lpClassName (Nom de la classe)
        title,                        // 3. lpWindowName (Titre de la fenêtre)
        styleFixe,                    // 4. dwStyle (Styles standards)
        CW_USEDEFAULT, CW_USEDEFAULT, // 5 & 6. X, Y (Position initiale)
        width, height,                // 7 & 8. nWidth, nHeight (Largeur, Hauteur)
        NULL,                         // 9. hWndParent (Fenêtre parente)
        NULL,                         // 10. hMenu (Menu)
        hinstance,                    // 11. hInstance (Instance de l'application)
        info                          // 12. lpParam (Données supplémentaires)
    );
}
int launchWindow(HWND hwnd)
{
    if (hwnd == NULL)
    {
        return 0;
    }

    // 4. Affichage de la fenêtre
    ShowWindow(hwnd, SW_SHOWDEFAULT);

    // 5. Boucle de messages principale
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (UINT)(msg.lParam);
}
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowDataInformation *window_info = (WindowDataInformation *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_CREATE:
    {
        LoadCache();

        CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
        // On récupère le pointeur qu'on a glissé dans CreateWindowEx
        window_info = (WindowDataInformation *)pCreate->lpCreateParams;

        // On l'attache définitivement à ce HWND précis
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window_info);

        return 0;
    }

    case WM_NOTIFY:
    {
        if (NotifListView(lParam))
            return 0;

        break;
    }

    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        for (int i = 0; i < window_info->n_buttons; i++)
        {
            if (ButtonHitTest(wParam, xPos, yPos, window_info->btnHead + i))
                return 0;
        }

        break;
    }

    case WM_LBUTTONDOWN:
    {
        // Récupération des coordonnées du clic
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        for (int i = 0; i < window_info->n_buttons; i++)
        {
            if (ButtonClickTest(wParam, xPos, yPos, window_info->btnHead + i))
                return 0;
        }

        break;
    }

    case WM_PAINT:
    {

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(mainHwnd, &ps);

        HFONT hOldFont = (HFONT)SelectObject(hdc, g_font);

        for (int i = 0; i < window_info->n_buttons; i++)
            ButtonDraw(hdc, window_info->btnHead + i);

        SelectObject(hdc, hOldFont);

        EndPaint(mainHwnd, &ps);

        return 0;
    }

    case WM_DESTROY:
    {
        SaveCache();
        PostQuitMessage(0);
        return 0;
    }
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}