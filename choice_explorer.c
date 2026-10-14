#include "choice_explorer.h"
#include "theme.h"
#include "backend.h"

#pragma comment(lib, "dwmapi.lib")

static Label g_labels[10];
static ExplorerInfo g_Explorers[10];
static int m_explorerCount = 0;
static int g_indexSelection = -1;
static HWND mhwnd;
static WNDPROC g_OldStaticProc = NULL;
static BOOL m_thumbnail_hover = FALSE;

#define ID_LISTCHOIX 205
#define MAX_EXPLORERS 10
#define ID_MIGNATURE_BASE 6000
#define THUMBNAIL_WIDTH 290
#define THUMBNAIL_HEIGHT 210
#define MARGIN 30
#define LABEL_MARGIN 10
#define THUMBNAIL_GAP 30
#define THUMBNAIL_LABEL_H 10
#define COLUMN_MAX 3

SearchResult SearchOpenExplorer(wchar_t* outPath, DWORD maxLen);
static inline void CreateClassRegistration(const wchar_t* classname, HINSTANCE hInst);
static inline RECT CalculateWindowRect();
static inline POINT CalculateThumbnailPos(int index);
static inline void RunModal();
static inline void OnThumbnailMouseMove(HWND hwnd, HWND parent);
static inline void CopyNamePath(wchar_t* destName, wchar_t* sourcePath);
static inline void CloseChoiceExplorer();
static inline void ShowThumbnailWindow(HWND hwndPopup, ExplorerInfo* explorer, RECT rectview);
static inline void AttachThumbnailProc(HWND thumbnail);
static inline void FillThumbnailLabel(int index, POINT thumbnailPos, wchar_t* text);

BOOL CreateChoiceExplorer(HWND hwndParent, wchar_t *outPath, DWORD maxLen)
{
    SearchResult sr = SearchOpenExplorer(outPath, maxLen);

    if (sr != SEARCH_RESULT_MULTIPLE)
        return sr != SEARCH_RESULT_NONE;

    g_indexSelection = -1;

    HINSTANCE hInst = GetModuleHandle(NULL);

    CreateClassRegistration(L"ModernAltTabClass", hInst);

    RECT win = CalculateWindowRect();
    DWORD popupStyle = WS_POPUP | WS_BORDER | WS_VISIBLE;

    HWND hwndPopup = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"ModernAltTabClass", L"Choix Dossier",
        popupStyle,
        win.left, win.top, win.right, win.bottom,
        hwndParent, NULL, hInst, NULL);

    if (hwndPopup != NULL)
    {
        ShowWindow(hwndPopup, SW_SHOW);
        UpdateWindow(hwndPopup);

        RunModal();
    }

    UnregisterClassW(L"ModernAltTabClass", hInst);

    if (g_indexSelection != -1)
    {
        wcsncpy_s(outPath, maxLen, g_Explorers[g_indexSelection].chemin, _TRUNCATE);
        return TRUE;
    }
    return FALSE;
}

static LRESULT CALLBACK ThumbnailSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int index = (int)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    HWND hwndParent = GetParent(hwnd);

    switch (msg)
    {
    case WM_MOUSEMOVE:
    {
        OnThumbnailMouseMove(hwnd, hwndParent);
        break;
    }
    case WM_MOUSELEAVE:
    {
        InvalidateRect(hwndParent, NULL, TRUE);
        m_thumbnail_hover = FALSE;
        break;
    }
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    {
        g_indexSelection = index;
        m_thumbnail_hover = FALSE;
        DestroyWindow(hwndParent);
        return 0;
    }
    }
    return CallWindowProcW(g_OldStaticProc, hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK AltTabChoixProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        HINSTANCE hInst = GetModuleHandle(NULL);

        for (int i = 0; i < m_explorerCount; i++)
        {
            POINT pos = CalculateThumbnailPos(i);

            wchar_t nomDossier[MAX_PATH] = {0};

            CopyNamePath(nomDossier, g_Explorers[i].chemin);

            HWND hwndPlace = CreateWindowExW(
                0, L"STATIC", L"",
                WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                pos.x, pos.y, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, hwnd, (HMENU)(INT_PTR)(ID_MIGNATURE_BASE + i), hInst, NULL);

            FillThumbnailLabel(i, pos, nomDossier);

            g_Explorers[i].hwndPlaceholder = hwndPlace;
            SetWindowLongPtrW(hwndPlace, GWLP_USERDATA, (LONG_PTR)i);

            AttachThumbnailProc(hwndPlace);

            RECT rcDest = { pos.x, pos.y, pos.x + THUMBNAIL_WIDTH + 2, pos.y + THUMBNAIL_HEIGHT+4 };

            ShowThumbnailWindow(hwnd, g_Explorers+i, rcDest);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        SetTextColor(hdc, g_colors[IC_TEXT]);
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, g_font);

        RECT rcTitle = {20, 10, 500, 35};
        DrawTextW(hdc, L"Sélectionnez la fenêtre de l'Explorateur à synchroniser :", -1, &rcTitle, DT_LEFT | DT_VCENTER);

        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);

        for (int i = 0; i < m_explorerCount; i++)
        {
            RECT rc;
            GetWindowRect(g_Explorers[i].hwndPlaceholder, &rc);
            MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);

            if (PtInRect(&rc, pt))
            {
                InflateRect(&rc, 3, 3);
                HPEN hOldPen = (HPEN)SelectObject(hdc, CONST_PEN_SELECTION);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
               
                Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

                SelectObject(hdc, hOldPen); 
                SelectObject(hdc, hOldBrush);
            }

            TextOutW(hdc, g_labels[i].x, g_labels[i].y, g_labels[i].text, lstrlenW(g_labels[i].text));
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hwnd);
        }
        break;
    }

    case WM_DESTROY:
        m_thumbnail_hover = FALSE;
        CloseChoiceExplorer();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

SearchResult SearchOpenExplorer(wchar_t* outPath, DWORD maxLen)
{
    m_explorerCount = Backend_GetOpenExplorers(g_Explorers, MAX_EXPLORERS);
    if (m_explorerCount == 0) {
        return SEARCH_RESULT_NONE;
    }
    else if (m_explorerCount == 1) {
        wcsncpy_s(outPath, maxLen, g_Explorers[0].chemin, _TRUNCATE);
        return SEARCH_RESULT_UNIQUE;
    }

    return SEARCH_RESULT_MULTIPLE;
}

static inline void FillThumbnailLabel(int index, POINT thumbnailPos, wchar_t* text) {
    g_labels[index].x = thumbnailPos.x;
    g_labels[index].y = thumbnailPos.y + THUMBNAIL_HEIGHT + LABEL_MARGIN;
    wcscpy_s(g_labels[index].text, MAX_PATH, text);
}

static inline void ShowThumbnailWindow(HWND hwndPopup, ExplorerInfo* explorer, RECT rectview)
{
    if (SUCCEEDED(DwmRegisterThumbnail(hwndPopup, explorer->hwndExplorateur, &explorer->hThumbnail)))
    {

        DWM_THUMBNAIL_PROPERTIES props = { 0 };

        props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_OPACITY;
        props.rcDestination = rectview;
        props.fVisible = TRUE;
        props.opacity = 255;

        DwmUpdateThumbnailProperties(explorer->hThumbnail, &props);
    }
}

static inline void CloseChoiceExplorer()
{
    for (int i = 0; i < m_explorerCount; i++)
    {
        if (g_Explorers[i].hThumbnail != NULL)
        {
            DwmUnregisterThumbnail(g_Explorers[i].hThumbnail);
            g_Explorers[i].hThumbnail = NULL;
        }
    }
}

static inline void CreateClassRegistration(const wchar_t* classname, HINSTANCE hInst) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = AltTabChoixProc;
    wc.hInstance = hInst;
    wc.hbrBackground = g_brushs[IC_WINDOW_COLOR];
    wc.lpszClassName = classname;
    wc.style = CS_DBLCLKS;
    RegisterClassW(&wc);
}

static inline POINT CalculateThumbnailPos(int index) {
    int col = index % COLUMN_MAX;
    int row = index / COLUMN_MAX;

    int x = MARGIN + col * (THUMBNAIL_WIDTH + THUMBNAIL_GAP);
    int y = MARGIN + row * (THUMBNAIL_HEIGHT + THUMBNAIL_GAP);
    y += THUMBNAIL_LABEL_H + LABEL_MARGIN;

    return (POINT) { x, y };
}

static inline RECT CalculateWindowRect() {
    int colonnes = (m_explorerCount > COLUMN_MAX) ? COLUMN_MAX : m_explorerCount;
    int lignes = (m_explorerCount + 2) / COLUMN_MAX;

    int winW = MARGIN + colonnes * (THUMBNAIL_WIDTH + THUMBNAIL_GAP);
    int winH = MARGIN + lignes * (THUMBNAIL_HEIGHT + THUMBNAIL_GAP);
    winH += THUMBNAIL_LABEL_H * 2 + LABEL_MARGIN;

    int posX = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

    return (RECT) { posX, posY, winW, winH };
}

static inline void RunModal() {
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

static inline void AttachThumbnailProc(HWND thumbnail) {
    if (!g_OldStaticProc)
    {
        g_OldStaticProc = (WNDPROC)GetWindowLongPtrW(thumbnail, GWLP_WNDPROC);
    }
    SetWindowLongPtrW(thumbnail, GWLP_WNDPROC, (LONG_PTR)ThumbnailSubclassProc);
}

static inline void OnThumbnailMouseMove(HWND hwnd, HWND parent) {
    if (!m_thumbnail_hover) {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);
    
        InvalidateRect(parent, NULL, TRUE);

        m_thumbnail_hover = TRUE;
    }
}

static inline void CopyNamePath(wchar_t* destName, wchar_t* sourcePath) {
    wchar_t* dernierSlash = wcsrchr(sourcePath, L'\\');

    if (!dernierSlash || dernierSlash[1] == L'\0')
    {
        wcsncpy_s(destName, MAX_PATH, sourcePath, _TRUNCATE);
    }
    else
    {
        wcsncpy_s(destName, MAX_PATH, dernierSlash + 1, _TRUNCATE);
    }
}