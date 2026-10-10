#include "pch.h"

typedef struct {
    int x, y;
    wchar_t text[MAX_PATH];
} Label;

typedef struct
{
    wchar_t chemin[MAX_PATH];
    HWND hwndExplorateur;  // Le handle (HWND) de la fenêtre Windows
    HTHUMBNAIL hThumbnail; // L'identifiant de flux vidéo généré par le DWM
    HWND hwndPlaceholder;
} ExplorateurOuvert;

void fill_g_Explorers();

BOOL create_choice_explorer(HWND hwndParent, wchar_t *outPath, DWORD maxLen);
LRESULT CALLBACK VignetteSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK AltTabChoixProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);