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


typedef enum {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
} PreferredAppMode;

// Les fonctions magiques de Windows pour le Dark Mode natif Win32
typedef PreferredAppMode(WINAPI* pSetPreferredAppMode)(PreferredAppMode appMode);
typedef BOOL(WINAPI* pAllowDarkModeForWindow)(HWND hwnd, BOOL allow);


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

    HMODULE hUxTheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxTheme)
    {
        // Ordinal 135 = SetPreferredAppMode
        pSetPreferredAppMode SetPreferredAppMode =
            (pSetPreferredAppMode)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(135));

        if (SetPreferredAppMode) {
            SetPreferredAppMode(ForceDark); // Force le mode sombre pour l'application
        }
        FreeLibrary(hUxTheme);
    }
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

    HMODULE hUxTheme = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxTheme)
    {
        // Ordinal 133 = AllowDarkModeForWindow
        pAllowDarkModeForWindow AllowDarkModeForWindow =
            (pAllowDarkModeForWindow)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(133));

        if (AllowDarkModeForWindow) {
            AllowDarkModeForWindow(hwnd, TRUE);
        }

        // Appliquer le thème pour forcer les Scrollbars à devenir sombres
        SetWindowTheme(hwnd, L"DarkMode_Explorer", NULL);

        FreeLibrary(hUxTheme);
    }

    //SetWindowTheme(hwnd, (isDarkMode ? L"DarkMode_Explorer" : L"Explorer"), NULL);
    SetWindowTheme(hwnd, L"DarkMode_ItemsView", NULL);
    HWND hHeader = ListView_GetHeader(hwnd);

    // 2. Lui appliquer le thème sombre de l'explorateur
    if (hHeader) {
        SetWindowTheme(hHeader, L"DarkMode_ItemsView", NULL); // Ou "DarkMode_Explorer"
    }
    /*HWND hwndHeader = ListView_GetHeader(hwnd);
    if (hwndHeader != NULL)
    {
        SetWindowTheme(hwndHeader, L"DarkMode_ItemsView", NULL);
    }*/
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