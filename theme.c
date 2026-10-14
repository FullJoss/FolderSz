#include "theme.h"
#include <commctrl.h>
#include <uxtheme.h>

#pragma comment(lib, "uxTheme.lib")
#pragma comment(lib, "comctl32.lib")


COLORREF g_colors[IC_COUNT] = { 0 };
HBRUSH g_brushs[IC_COUNT] = { 0 };
HFONT g_font;

HPEN CONST_PEN_SELECTION = NULL;

BOOL isDarkMode = FALSE;

static inline void FillVarColorDark() {
    g_colors[IC_WINDOW_COLOR] = RGB(25, 25, 25);

    g_colors[IC_BTN_BACKGROUND] = RGB(30, 30, 30);
    g_colors[IC_BTN_HOVER] = RGB(45, 45, 45);
    g_colors[IC_BTN_CLICK] = RGB(60, 60, 60);
    g_colors[IC_BTN_BORDER] = RGB(85, 85, 85);
    g_colors[IC_BTN_HOVER_BORDER] = RGB(220, 220, 220);
    g_colors[IC_TEXT] = RGB(255, 255, 255);
}

static inline void FillVarColorLight() {
    g_colors[IC_WINDOW_COLOR] = RGB(248, 250, 252);

    g_colors[IC_BTN_BACKGROUND] = RGB(245, 245, 245);
    g_colors[IC_BTN_HOVER] = RGB(230, 230, 230);
    g_colors[IC_BTN_CLICK] = RGB(200, 200, 200);
    g_colors[IC_BTN_BORDER] = RGB(180, 180, 180);
    g_colors[IC_BTN_HOVER_BORDER] = RGB(0, 120, 125);
    g_colors[IC_TEXT] = RGB(0, 0, 0);
}

static inline void FillBrushWithColor() {
    for (int i_color = 0; i_color < IC_COUNT; i_color++)
    {
        g_brushs[i_color] = CreateSolidBrush(g_colors[i_color]);
    }
}

static inline void FreeBrushs() {
    for (int i_color = 0; i_color < IC_COUNT; i_color++)
    {
        DeleteObject(g_brushs[i_color]);
    }
}

static HFONT CreateCustomFont(const wchar_t* fontName, int size, int weight, BOOL useItalic)
{
    HDC hdc = GetDC(NULL);                     
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    int fontHeight = -MulDiv(size, dpiY, 72);

    return CreateFontW(
        fontHeight,                 
        0,                           
        0,                          
        0,                          
        weight,                  
        useItalic,                    
        FALSE,                       
        FALSE,                      
        DEFAULT_CHARSET,            
        OUT_DEFAULT_PRECIS,         
        CLIP_DEFAULT_PRECIS,        
        CLEARTYPE_QUALITY,          
        DEFAULT_PITCH | FF_DONTCARE,
        fontName
    );
}

static BOOL IsDarkMode()
{
    DWORD value = 1;

    HKEY hKey;
    DWORD size = sizeof(value);

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return (value == 0);
}

void InitTheme()
{
    g_font = CreateCustomFont(L"Segoe UI", 9, FW_MEDIUM, FALSE);

    if ((isDarkMode = IsDarkMode()))
    {
        FillVarColorDark();
    }
    else {
        FillVarColorLight();
    }

    CONST_PEN_SELECTION = CreatePen(PS_SOLID, 3, RGB(0, 120, 225));

    FillBrushWithColor();
}

void FreeTheme()
{
    DeleteObject(g_font);
    DeleteObject(CONST_PEN_SELECTION);
    FreeBrushs();
}

void SetListViewTheme(HWND hwnd)
{
    ListView_SetBkColor(hwnd, g_colors[IC_WINDOW_COLOR]);
    ListView_SetTextBkColor(hwnd, g_colors[IC_WINDOW_COLOR]);
    ListView_SetTextColor(hwnd, g_colors[IC_TEXT]);

    SetWindowTheme(hwnd, (isDarkMode ? L"DarkMode_Explorer" : L"Explorer"), NULL);

    HWND hwndHeader = ListView_GetHeader(hwnd);
    if (hwndHeader != NULL)
    {
        SetWindowTheme(hwndHeader, L"DarkMode_ItemsView", NULL);
    }
}

void SetTitleWindowTheme(HWND hWnd) {

    const int DW_NEW_ATTRIB_WIN10_WIN11 = 20;
    HRESULT hr = DwmSetWindowAttribute(hWnd, DW_NEW_ATTRIB_WIN10_WIN11, &isDarkMode, sizeof(BOOL));

    if (FAILED(hr)) {
        const int DW_OLD_ATTRIB_WIN7_WIN10 = 19;
        DwmSetWindowAttribute(hWnd, DW_OLD_ATTRIB_WIN7_WIN10, &isDarkMode, sizeof(BOOL));
    }

    // Update Display
    SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}