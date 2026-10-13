#pragma once
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
} ExplorerInfo;

typedef enum {
    SEARCH_RESULT_NONE,
    SEARCH_RESULT_UNIQUE,
    SEARCH_RESULT_MULTIPLE
} SearchResult;

SearchResult CreateChoiceExplorer(HWND hwndParent, wchar_t *outPath, DWORD maxLen);
LRESULT CALLBACK AltTabChoixProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);