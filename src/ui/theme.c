/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#include "theme.h"
#include <commctrl.h>
#include <uxtheme.h>

#pragma comment(lib, "uxTheme.lib")
#pragma comment(lib, "comctl32.lib")


COLORREF g_colors[IC_COUNT] = { 0 };
HBRUSH g_brushs[IC_COUNT] = { 0 };
HFONT g_font;
int g_fontHeight = 0;
HIMAGELIST g_systemImageList = NULL;
HPEN CONST_PEN_SELECTION = NULL;
BOOL isDarkMode = FALSE;

static inline void FreeBrushs();
static inline void FillVarColorDark();
static inline void FillVarColorLight();
static inline void FillBrushWithColor();
static inline BOOL IsDarkMode();
static inline void GetAccessHSystemList();
static HFONT CreateCustomFont(const wchar_t* fontName, int size, int weight, BOOL useItalic);
static void SyncColorTheme();

void InitTheme()
{
    GetAccessHSystemList();

    g_font = CreateCustomFont(L"Segoe UI", 10, FW_MEDIUM, FALSE);

    SyncColorTheme();
    FillBrushWithColor();

    CONST_PEN_SELECTION = CreatePen(PS_SOLID, 3, RGB(0, 120, 225));
}

void FreeTheme()
{
    DeleteObject(g_font);
    DeleteObject(CONST_PEN_SELECTION);
    FreeBrushs();
}

void ResyncColorTheme() {
    FreeBrushs();
    SyncColorTheme();
    FillBrushWithColor();
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



static HFONT CreateCustomFont(const wchar_t* fontName, int size, int weight, BOOL useItalic)
{
    HDC hdc = GetDC(NULL);

    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);

    ReleaseDC(NULL, hdc);

    int fontHeight = -MulDiv(size, dpiY, 72);
    g_fontHeight = fontHeight;

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


static inline BOOL IsDarkMode()
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

static inline void GetAccessHSystemList() {
    if (g_systemImageList) return;
    SHFILEINFOW sfi = { 0 };
    // SHGFI_SMALLICON : 16x16 pixels.
    g_systemImageList = (HIMAGELIST)SHGetFileInfoW(
        L"C:\\",
        FILE_ATTRIBUTE_DIRECTORY,
        &sfi,
        sizeof(sfi),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
    );
}

static void SyncColorTheme() {
    isDarkMode = IsDarkMode();

    if (isDarkMode)
        FillVarColorDark();
    else
        FillVarColorLight();
}

static inline void FillVarColorDark() {

    g_colors[IC_WINDOW_COLOR] = RGB(28, 28, 28);    
    g_colors[IC_BTN_BACKGROUND] = RGB(40, 40, 40);  
    g_colors[IC_BTN_HOVER] = RGB(45, 45, 45);
    g_colors[IC_BTN_CLICK] = RGB(35, 35, 35);
    g_colors[IC_BTN_BORDER] = RGB(70, 70, 70); 
    g_colors[IC_BTN_HOVER_BORDER] = RGB(0, 125, 237);

    g_colors[IC_LV_BACKGROUND] = RGB(28, 28, 28);     
    g_colors[IC_LV_BORDER] = RGB(43, 43, 43);     
    g_colors[IC_LV_TRACK] = RGB(32, 32, 32);  
    g_colors[IC_LV_THUMB] = RGB(74, 74, 74); 
    g_colors[IC_LV_THUMB_DRAG] = RGB(0, 120, 212);  
    g_colors[IC_LV_ROW_SELECT] = RGB(45, 45, 45);
    g_colors[IC_LV_ROW_SELECT_BORDER] = RGB(0, 125, 237);
    g_colors[IC_LV_ROW_HOVER] = RGB(38, 38, 38);    
    g_colors[IC_LV_HEAD] = RGB(32, 32, 32);   
    g_colors[IC_LV_HEAD_HOVER] = RGB(40, 40, 40); 



    g_colors[IC_TEXT] = RGB(230, 230, 240);
}

static inline void FillVarColorLight() {

    g_colors[IC_WINDOW_COLOR] = RGB(255, 255, 255);
    g_colors[IC_BTN_BACKGROUND] = RGB(245, 245, 247); 
    g_colors[IC_BTN_HOVER] = RGB(235, 235, 240);
    g_colors[IC_BTN_CLICK] = RGB(225, 225, 230); 
    g_colors[IC_BTN_BORDER] = RGB(220, 220, 225); 
    g_colors[IC_BTN_HOVER_BORDER] = RGB(160, 160, 170); 

    g_colors[IC_LV_BACKGROUND] = RGB(255, 255, 255); 
    g_colors[IC_LV_BORDER] = RGB(225, 225, 230);
    g_colors[IC_LV_TRACK] = RGB(245, 245, 247);  
    g_colors[IC_LV_THUMB] = RGB(200, 200, 205); 
    g_colors[IC_LV_THUMB_DRAG] = RGB(140, 140, 145); 
    g_colors[IC_LV_ROW_SELECT] = RGB(230, 242, 255); 
    g_colors[IC_LV_ROW_SELECT_BORDER] = RGB(0, 125, 237);
    g_colors[IC_LV_ROW_HOVER] = RGB(245, 247, 250);  
    g_colors[IC_LV_HEAD] = RGB(245, 245, 247);  
    g_colors[IC_LV_HEAD_HOVER] = RGB(235, 235, 240);

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
