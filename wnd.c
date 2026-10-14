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

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = L"WINDOW_SIMPLE";
    wc.hbrBackground = g_brushs[IC_WINDOW_COLOR];
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(hinstance, MAKEINTRESOURCEW(1));

    RegisterClassW(&wc);

    const POINT INITIAL_WINDOW_POS = { CW_USEDEFAULT , CW_USEDEFAULT };

    DWORD styleFixe = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    RECT rc = { 0, 0, width, height };
    DWORD dwExStyle = 0;

    AdjustWindowRectEx(&rc, styleFixe, FALSE, dwExStyle);

    int largeurGlobale = rc.right - rc.left;
    int hauteurGlobale = rc.bottom - rc.top;

    HWND temp = CreateWindowExW(
        dwExStyle,
        L"WINDOW_SIMPLE",            
        title,                      
        styleFixe,                   
        INITIAL_WINDOW_POS.x, INITIAL_WINDOW_POS.y,
        largeurGlobale, hauteurGlobale,
        NULL,                      
        NULL,                 
        hinstance,                  
        info                      
    );

    if (!temp) return NULL;

    SetTitleWindowTheme(temp);

    return temp;
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

    HandleListViewMouseMove(mainHwnd, xPos, yPos, window_info->lfv);

    int r = 1;
    for (int i = 0; i < window_info->n_buttons; i++)
    {
        if (ButtonHitTest(wParam, xPos, yPos, window_info->buttons + i))
            r = 0;
    }

    return r;
}
static inline int OnLeftButtonDown(WPARAM wParam, LPARAM lParam, WindowDataInformation* window_info) {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    HandleListViewLButtonDown(mainHwnd, xPos, yPos, window_info->lfv);

    for (int i = 0; i < window_info->n_buttons; i++)
    {
        if (ButtonClickTest(wParam, xPos, yPos, window_info->buttons + i))
            return 0;
    }

    return 1;
}
static inline int OnPaint(WindowDataInformation* window_info) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(mainHwnd, &ps);

    HFONT hOldFont = (HFONT)SelectObject(hdc, g_font);

    DrawListFileView(hdc, window_info->lfv, ps.rcPaint);

    for (int i = 0; i < window_info->n_buttons; i++)
        ButtonDraw(hdc, window_info->buttons + i);

    SelectObject(hdc, hOldFont);
    EndPaint(mainHwnd, &ps);

    return 0;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowDataInformation *window_info = (WindowDataInformation *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    static BOOL trackingMouse = FALSE;

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

    case WM_MOUSEMOVE:
    {
        if (!trackingMouse) {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            tme.dwHoverTime = HOVER_DEFAULT;

            if (TrackMouseEvent(&tme)) {
                trackingMouse = TRUE;
            }
        }

        if (OnMouseMove(wParam, lParam, window_info) == 0)
            return 0;

        break;
    }
    case WM_MOUSELEAVE: {
        trackingMouse = FALSE;

        for (int i = 0; i < window_info->n_buttons; i++) {
            window_info->buttons[i].state = PBS_NORMAL;
        }

        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }

    case WM_MOUSEWHEEL:
    {
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        HandleListViewMouseWheel(hwnd, wheelDelta, lParam, window_info->lfv);
        break;
    }

    case WM_LBUTTONDOWN:
    {
        if (OnLeftButtonDown(wParam, lParam, window_info) == 0)
            return 0;

        break;
    }

    case WM_LBUTTONUP:
    {
        HandleListViewLButtonUp(hwnd, window_info->lfv);
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