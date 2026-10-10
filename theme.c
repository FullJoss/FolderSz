#include "theme.h"

COLORREF btn_colors[5] = {
    // lightmode
    RGB(245, 245, 245), // Background
    RGB(230, 230, 230), // hover
    RGB(200, 200, 200), // click
    RGB(180, 180, 180), // border
    RGB(0, 0, 0)        // Text
};

HBRUSH btn_brushs[4] = {0};
HBRUSH window_brush;
HFONT g_font;

BOOL is_dark_mode = FALSE;

static HFONT CreateSegoeUI(int size)
{
    HDC hdc = GetDC(NULL);                     // Récupère le contexte de l'écran pour obtenir le DPI
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY); // Généralement 96 sur un écran classique (100%)
    ReleaseDC(NULL, hdc);

    // Conversion de 12 points en unités logiques (pixels)
    int fontHeight = -MulDiv(size, dpiY, 72);

    return CreateFontW(
        fontHeight,                  // 1. nHeight : Hauteur des lettres en pixels
        0,                           // 2. nWidth : Largeur moyenne (0 = automatique proportionnel)
        0,                           // 3. nEscapement : Angle d'écriture (0 = horizontal)
        0,                           // 4. nOrientation : Angle des caractères
        FW_MEDIUM,                   // 5. fnWeight : Épaisseur (FW_LIGHT, FW_NORMAL, FW_SEMIBOLD, FW_BOLD)
        FALSE,                       // 6. fdwItalic : Italique (TRUE/FALSE)
        FALSE,                       // 7. fdwUnderline : Souligné (TRUE/FALSE)
        FALSE,                       // 8. fdwStrikeOut : Barré (TRUE/FALSE)
        DEFAULT_CHARSET,             // 9. fdwCharSet : Jeu de caractères
        OUT_DEFAULT_PRECIS,          // 10. fdwOutputPrecision : Précision de sortie
        CLIP_DEFAULT_PRECIS,         // 11. fdwClipPrecision : Précision de découpage
        CLEARTYPE_QUALITY,           // 12. fdwQuality : LE SECRET DU REFLÈTEMENT NET (Force l'antialiasing ClearType)
        DEFAULT_PITCH | FF_DONTCARE, // 13. fdwPitchAndFamily : Type de espacement
        L"Segoe UI"                  // 14. lpszFace : Le nom de la police système
    );
}
static int isDarkMode()
{
    HKEY hKey;
    DWORD value = 1; // Par défaut, mode clair si la clé n'existe pas
    DWORD size = sizeof(value);

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, NULL, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return (value == 0); // Si AppsUseLightTheme vaut 0, le mode sombre est activé
}
void init_theme()
{
    window_brush = (HBRUSH)(COLOR_WINDOW + 1);
    if ((is_dark_mode = isDarkMode()))
    {
        // darkmode
        btn_colors[0] = RGB(30, 30, 30);    // Background
        btn_colors[1] = RGB(45, 45, 45);    // hover
        btn_colors[2] = RGB(60, 60, 60);    // click
        btn_colors[3] = RGB(85, 85, 85);    // border
        btn_colors[4] = RGB(255, 255, 255); // Text

        window_brush = CreateSolidBrush(btn_colors[0]);
    }

    for (int i = 0; i < 4; i++)
    {
        btn_brushs[i] = CreateSolidBrush(btn_colors[i]);
    }

    g_font = CreateSegoeUI(9);
}
void close_theme()
{
    DeleteObject(g_font);

    for (int i = 0; i < 4; i++)
    {
        DeleteObject(btn_brushs[i]);
    }

    if (is_dark_mode)
        DeleteObject(window_brush);
}

void set_listview_theme(HWND mhwnd)
{
    COLORREF bg_color = is_dark_mode ? RGB(32, 32, 32) : RGB(240, 240, 240);

    ListView_SetBkColor(mhwnd, bg_color);        // Couleur de fond générale
    ListView_SetTextBkColor(mhwnd, bg_color);    // Couleur de fond derrière le texte
    ListView_SetTextColor(mhwnd, btn_colors[4]); // Couleur du texte

    if (is_dark_mode)
    {
        // 1. Récupérer le HWND de l'en-tête de la ListView
        HWND hWndHeader = (HWND)SendMessage(mhwnd, LVM_GETHEADER, 0, 0);

        if (hWndHeader != NULL) {
            // 2. Lui appliquer le thème sombre natif pour les listes sous Windows 10/11
            SetWindowTheme(hWndHeader, L"DarkMode_ItemsView", NULL);
        }

        SetWindowTheme(mhwnd, L"DarkMode_Explorer", NULL);
    }
}


void ActiverDarkModeUltra(HWND hWnd) {

    // 1. Essai avec la constante moderne officielle (Windows 11 et Win 10 récents -> valeur 20)
    HRESULT hr = DwmSetWindowAttribute(hWnd, 20, &is_dark_mode, sizeof(BOOL));

    // 2. Si ça échoue, essai avec l'ancienne constante masquée (Anciens Windows 10 -> valeur 19)
    if (FAILED(hr)) {
        DwmSetWindowAttribute(hWnd, 19, &is_dark_mode, sizeof(BOOL));
    }

    // 3. Forcer Windows à redessiner immédiatement le cadre de la fenêtre (Résout 90% des "ça marche pas")
    SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}