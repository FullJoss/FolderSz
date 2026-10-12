#include "wnd.h"
#include "theme.h"
#include "feature.h"
#include "dialog.h"
#include "Button.h"
#include "list_view.h"
#include "cache.h"

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND CreateMainWindow(const wchar_t *title, int width, int height, WindowDataInformation *info)
{
    HINSTANCE hinstance = GetModuleHandle(NULL);

    DWORD styleFixe = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = L"WINDOW_SIMPLE";
    wc.hbrBackground = g_brushs[IC_WINDOW_COLOR];
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(hinstance, MAKEINTRESOURCEW(1));

    RegisterClassW(&wc);

    const POINT INITIAL_WINDOW_POS = { CW_USEDEFAULT , CW_USEDEFAULT };

    // 3. Création de la fenêtre avec CreateWindowEx
    return CreateWindowExW(
        0,                           
        L"WINDOW_SIMPLE",            
        title,                      
        styleFixe,                   
        INITIAL_WINDOW_POS.x, INITIAL_WINDOW_POS.y,
        width, height,              
        NULL,                      
        NULL,                 
        hinstance,                  
        info                      
    );
}
int LaunchWindow(HWND hwnd)
{
    if (hwnd == NULL)
    {
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (UINT)(msg.lParam);
}

static inline int OnMouseMove(WPARAM wParam, LPARAM lParam, WindowDataInformation* window_info) {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    for (int i = 0; i < window_info->n_buttons; i++)
    {
        if (ButtonHitTest(wParam, xPos, yPos, window_info->btnsHead + i))
            return 0;
    }

    return 1;
}
static inline int OnLeftButtonDown(WPARAM wParam, LPARAM lParam, WindowDataInformation* window_info) {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    for (int i = 0; i < window_info->n_buttons; i++)
    {
        if (ButtonClickTest(wParam, xPos, yPos, window_info->btnsHead + i))
            return 0;
    }

    return 1;
}
static inline int OnPaint(WindowDataInformation* window_info) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(mainHwnd, &ps);

    HFONT hOldFont = (HFONT)SelectObject(hdc, g_font);

    for (int i = 0; i < window_info->n_buttons; i++)
        ButtonDraw(hdc, window_info->btnsHead + i);

    SelectObject(hdc, hOldFont);
    EndPaint(mainHwnd, &ps);

    return 0;
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
       
        window_info = (WindowDataInformation *)pCreate->lpCreateParams;

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
        if (OnMouseMove(wParam, lParam, window_info) == 0)
            return 0;

        break;
    }

    case WM_LBUTTONDOWN:
    {
        if (OnLeftButtonDown(wParam, lParam, window_info) == 0)
            return 0;

        break;
    }

    case WM_PAINT:
    {
        OnPaint(window_info);
        break;
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