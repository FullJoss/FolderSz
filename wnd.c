#include "wnd.h"
#include "theme.h"
#include "feature.h"
#include "dialog.h"
#include "Button.h"
#include "list_view.h"
#include "cache.h"

static HDC hdcMem = NULL;
static HBITMAP hBmp = NULL;
static HBITMAP hBmpOld = NULL;
static RECT background = { 0 };

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static inline void FreeDoubleBuffering();
static inline void EnsureDoubleBuffering(HDC hdc, int w, int h);

HWND CreateMainWindow(const wchar_t *title, WindowDataInformation *info)
{
    HINSTANCE hinstance = GetModuleHandle(NULL);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hinstance;
    wc.hbrBackground = g_brushs[IC_WINDOW_COLOR];
    wc.lpszClassName = L"WINDOW_SIMPLE";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(hinstance, MAKEINTRESOURCEW(1));

    RegisterClassW(&wc);

    const POINT INITIAL_WINDOW_POS = { CW_USEDEFAULT , CW_USEDEFAULT };

    DWORD styleFixe = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    RECT rc = { 0, 0, info->width, info->height };
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

static inline int OnMouseMove(HWND hwnd, WPARAM wParam, LPARAM lParam, WindowDataInformation* window_info) {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    HandleListViewMouseMove(hwnd, xPos, yPos, window_info->lfv);

    int r = 1;
    for (int i = 0; i < window_info->n_buttons; i++)
    {
        if (ButtonHitTest(hwnd, wParam, xPos, yPos, window_info->buttons + i))
            r = 0;
    }

    return r;
}
static inline int OnLeftButtonDown(HWND hwnd, WPARAM wParam, LPARAM lParam, WindowDataInformation* window_info) {
    int xPos = GET_X_LPARAM(lParam);
    int yPos = GET_Y_LPARAM(lParam);

    if (PtInRect(&window_info->lfv->bounds, (POINT) { xPos, yPos }))
    {
        HandleListViewLButtonDown(hwnd, xPos, yPos, window_info->lfv);
    }

    for (int i = 0; i < window_info->n_buttons; i++)
    {
        if (ButtonClickTest(hwnd, wParam, xPos, yPos, window_info->buttons + i, window_info))
            return 0;
    }

    return 1;
}
static inline int OnPaint(HWND hwnd, WindowDataInformation* window_info) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    EnsureDoubleBuffering(hdc, window_info->width, window_info->height);

    FillRect(hdcMem, &background, g_brushs[IC_WINDOW_COLOR]);

    HFONT hOldFont = (HFONT)SelectObject(hdcMem, g_font);

    DrawListFileView(hdcMem, window_info->lfv, ps.rcPaint);

    for (int i = 0; i < window_info->n_buttons; i++)
        ButtonDraw(hdcMem, window_info->buttons + i);

    BitBlt(hdc, 0, 0, window_info->width, window_info->height, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOldFont);
    EndPaint(hwnd, &ps);

    return 0;
}

static inline void EnsureDoubleBuffering(HDC hdc, int width, int height) {
    if (!hdcMem) hdcMem = CreateCompatibleDC(hdc);
    if (!hBmp) {
        background.right = width;
        background.bottom = height;
        hBmp = CreateCompatibleBitmap(hdc, width, height);
        hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmp);
    }
}

static inline void FreeDoubleBuffering() {
    SelectObject(hdcMem, hBmpOld);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
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

        if (OnMouseMove(hwnd, wParam, lParam, window_info) == 0)
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
        if (OnLeftButtonDown(hwnd, wParam, lParam, window_info) == 0)
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
        OnPaint(hwnd, window_info);
        break;
    }

    case WM_DESTROY:
    {
        SaveCache();
        FreeDoubleBuffering();
        PostQuitMessage(0);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}