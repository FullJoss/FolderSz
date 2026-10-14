#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include "dialog.h"

// Configuration visuelle du mode sombre
#define ROW_HEIGHT          26
#define COLUMN_0_WIDTH      300
#define ICON_SIZE           16
#define ICON_PADDING        6
#define TEXT_PADDING        10
#define SCROLLBAR_WIDTH     14  // Une barre fine et moderne de 12px

BOOL g_TriCroissant = TRUE;
ColumnListView g_sortColumn = COLUMN_LISTVIEW_NAME;

HIMAGELIST hSystemImageList = NULL;

static HWND m_hwnd = NULL;
static ListFile* m_list;

static int CompareBySize(const void* a, const void* b);
static int CompareByName(const void* a, const void* b);
static inline void ChangeSortingMethod(ColumnListView clickedColumn);
static inline int CompareByDirectory(ElementFile* A, ElementFile* B);
static inline void SortAt(ColumnListView clickedColumn);
static inline void GetAccessHSystemList();
RECT GetScrollThumbRect(ListFileView* listView);

void DrawListFileView(HDC hdc, ListFileView* lfv) {
    ListFile *list = lfv->data;
    if (!list) return;

    if (!hSystemImageList)
        GetAccessHSystemList();


    // 1. Découpage de sécurité (Anti-débordement)
    HRGN hClipRegion = CreateRectRgnIndirect(&lfv->bounds);
    SelectClipRgn(hdc, hClipRegion);

    // 2. Dessiner le fond global (Noir/Gris très sombre)
    HBRUSH hBgBrush = CreateSolidBrush(RGB(30, 30, 30));
    FillRect(hdc, &lfv->bounds, hBgBrush);
    DeleteObject(hBgBrush);

    // Calcul de la zone réservée aux fichiers (on retire la largeur de la scrollbar à droite)
    RECT fileArea = lfv->bounds;
    fileArea.right -= SCROLLBAR_WIDTH;

    int totalContentHeight = list->count* ROW_HEIGHT;
    int viewHeight = lfv->bounds.bottom - lfv->bounds.top;

    // 3. DESSIN DES FICHIERS (Uniquement dans fileArea)
    if (list->count > 0 && list->file) {
        SetBkMode(hdc, TRANSPARENT);
        RECT rowBounds;
        RECT textBounds;
        wchar_t sizeBuffer[32];

        for (int i = 0; i < list->count; i++) {
            rowBounds.left = fileArea.left;
            rowBounds.right = fileArea.right;
            rowBounds.top = fileArea.top + (i * ROW_HEIGHT) - lfv->scrollPos;
            rowBounds.bottom = rowBounds.top + ROW_HEIGHT;

            if (rowBounds.bottom < fileArea.top) continue;
            if (rowBounds.top > fileArea.bottom) break;

            // Colonne 0 : Icône + Nom
            int iconY = rowBounds.top + ((ROW_HEIGHT - ICON_SIZE) / 2);
            ImageList_Draw(hSystemImageList, list->file[i].indexIcon, hdc, fileArea.left + 5, iconY, ILD_TRANSPARENT);

            textBounds.left = fileArea.left + 5 + ICON_SIZE + 6;
            textBounds.top = rowBounds.top;
            textBounds.right = fileArea.left + COLUMN_0_WIDTH;
            textBounds.bottom = rowBounds.bottom;
            SetTextColor(hdc, RGB(240, 240, 240));
            DrawTextW(hdc, list->file[i].name, -1, &textBounds, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            // Colonne 1 : Taille
            textBounds.left = fileArea.left + COLUMN_0_WIDTH + 10;
            textBounds.right = fileArea.right - 5;
            StrFormatByteSizeW(list->file[i].size, sizeBuffer, ARRAYSIZE(sizeBuffer));
            SetTextColor(hdc, RGB(150, 150, 150));
            DrawTextW(hdc, sizeBuffer, -1, &textBounds, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    // 4. DESSIN DE LA SCROLLBAR SUR MESURE (Rectangle à droite)
    if (totalContentHeight > viewHeight) { // On ne la dessine que si ça déborde
        RECT sbTrack = { fileArea.right, lfv->bounds.top, lfv->bounds.right, lfv->bounds.bottom };

        // Fond de la glissière (un peu plus clair que le fond global)
        HBRUSH hTrackBrush = CreateSolidBrush(RGB(37, 37, 37));
        FillRect(hdc, &sbTrack, hTrackBrush);
        DeleteObject(hTrackBrush);

        // Calcul de la taille et de la position du Curseur (Thumb)
        int thumbHeight = (viewHeight * viewHeight) / totalContentHeight;
        if (thumbHeight < 20) thumbHeight = 20; // Sécurité hauteur minimale pour le choper au clic

        int maxScrollPos = totalContentHeight - viewHeight;
        int maxThumbY = viewHeight - thumbHeight;
        int thumbTop = sbTrack.top + (lfv->scrollPos * maxThumbY) / maxScrollPos;

        RECT sbThumb = { sbTrack.left + 2, thumbTop, sbTrack.right - 2, thumbTop + thumbHeight };

        // Couleur du curseur (Gris mode sombre, style Windows 11)
        HBRUSH hThumbBrush = CreateSolidBrush(lfv->isDraggingScroll ? RGB(100, 100, 100) : RGB(70, 70, 70));
        FillRect(hdc, &sbThumb, hThumbBrush);
        DeleteObject(hThumbBrush);
    }

    SelectClipRgn(hdc, NULL);
    DeleteObject(hClipRegion);
}

void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* listView) {
    RECT thumbRect = GetScrollThumbRect(listView);
    POINT pt = { x, y };

    if (PtInRect(&thumbRect, pt)) {
        listView->isDraggingScroll = TRUE;
        listView->dragYOffset = y - thumbRect.top; // Conserver le décalage exact du clic
        SetCapture(hWnd); // Capturer la souris pour pouvoir glisser même en dehors de la fenêtre
        InvalidateRect(hWnd, &listView->bounds, FALSE);
    }
}

void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* listView) {
    if (!listView->isDraggingScroll) return;

    int viewHeight = listView->bounds.bottom - listView->bounds.top;
    int totalContentHeight = listView->data->count * ROW_HEIGHT;
    int maxScrollPos = totalContentHeight - viewHeight;

    RECT thumbRect = GetScrollThumbRect(listView);
    int thumbHeight = thumbRect.bottom - thumbRect.top;
    int maxThumbY = viewHeight - thumbHeight;

    // Calculer la nouvelle position cible du haut du curseur
    int proposedThumbTop = y - listView->dragYOffset - listView->bounds.top;

    // Borner les valeurs pour éviter de sortir des limites de la piste
    if (proposedThumbTop < 0) proposedThumbTop = 0;
    if (proposedThumbTop > maxThumbY) proposedThumbTop = maxThumbY;

    // Règle de trois pour convertir la position du curseur (pixels) en position de défilement du contenu
    int newScrollPos = (proposedThumbTop * maxScrollPos) / maxThumbY;

    if (newScrollPos != listView->scrollPos) {
        listView->scrollPos = newScrollPos;
        InvalidateRect(hWnd, &listView->bounds, FALSE); // Redessiner immédiatement sans scintillement
    }
}

void HandleListViewLButtonUp(HWND hWnd, ListFileView* listView) {
    if (listView->isDraggingScroll) {
        listView->isDraggingScroll = FALSE;
        ReleaseCapture();
        InvalidateRect(hWnd, &listView->bounds, FALSE);
    }
}

void HandleListViewMouseWheel(HWND hWnd, int wheelDelta, LPARAM lParam, ListFileView* listView) {
    if (!listView) return;

    // 1. Récupérer les coordonnées écran de la souris
    POINT mousePt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // Convertir les coordonnées écran en coordonnées relatives à la fenêtre (hWnd)
    ScreenToClient(hWnd, &mousePt);

    // 2. SÉCURITÉ : On ne scroll que si la souris est physiquement AU-DESSUS du composant
    if (!PtInRect(&listView->bounds, mousePt)) {
        return;
    }

    // Calculer le nombre d'octants de rotation (généralement 1 par cran de molette)
    int scrollCrans = wheelDelta / WHEEL_DELTA;

    // Définir la sensibilité : combien de lignes on saute par cran de molette (traditionnellement 3)
    int linesToScroll = 3;
    int pixelScrollAmount = scrollCrans * linesToScroll * ROW_HEIGHT;

    // 4. Mettre à jour la position (La molette est inversée : rouler vers le haut = défiler vers le haut)
    int newScrollPos = listView->scrollPos - pixelScrollAmount;

    // 5. Calculer les limites pour ne pas scroller dans le vide
    int viewHeight = listView->bounds.bottom - listView->bounds.top;
    int totalContentHeight = listView->data->count * ROW_HEIGHT;
    int maxScrollPos = totalContentHeight - viewHeight;
    if (maxScrollPos < 0) maxScrollPos = 0; // Le contenu tient entièrement dans l'écran

    // Borner la nouvelle position
    if (newScrollPos < 0) newScrollPos = 0;
    if (newScrollPos > maxScrollPos) newScrollPos = maxScrollPos;

    // 6. Si la position a changé, on redessine le composant
    if (newScrollPos != listView->scrollPos) {
        listView->scrollPos = newScrollPos;
        InvalidateRect(hWnd, &listView->bounds, FALSE); // FALSE évite le scintillement (flicker)
    }
}


RECT GetScrollThumbRect(ListFileView* listView) {
    int viewHeight = listView->bounds.bottom - listView->bounds.top;
    int totalContentHeight = listView->data->count * ROW_HEIGHT;
    RECT thumbRect = { 0 };

    if (totalContentHeight <= viewHeight) return thumbRect;

    int thumbHeight = (viewHeight * viewHeight) / totalContentHeight;
    if (thumbHeight < 20) thumbHeight = 20;

    int maxScrollPos = totalContentHeight - viewHeight;
    int maxThumbY = viewHeight - thumbHeight;
    int thumbTop = listView->bounds.top + (listView->scrollPos * maxThumbY) / maxScrollPos;

    thumbRect.left = listView->bounds.right - SCROLLBAR_WIDTH + 2;
    thumbRect.right = listView->bounds.right - 2;
    thumbRect.top = thumbTop;
    thumbRect.bottom = thumbTop + thumbHeight;

    return thumbRect;
}


void UpdateListFileView(ListFileView* lfv)
{
    qsort(lfv->data->file, lfv->data->count, sizeof(ElementFile), CompareByName);
    InvalidateRect(lfv->hwnd, &lfv->bounds, FALSE);
}

static int CompareByName(const void* a, const void* b)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    diff = _wcsicmp(elemA->name, elemB->name); // case-insensitive
    return g_TriCroissant ? diff : -diff;
}

static int CompareBySize(const void* a, const void* b)
{
    ElementFile* elemA = (ElementFile*)a;
    ElementFile* elemB = (ElementFile*)b;

    int diff = CompareByDirectory(elemA, elemB);
    if (diff != 0)
        return diff;

    BOOL ABetter = (elemA->size) > (elemB->size);
    BOOL BBetter = (elemA->size) < (elemB->size);
    diff = ABetter - BBetter;
    return g_TriCroissant ? diff : -diff;
}

static inline void GetAccessHSystemList() {
    SHFILEINFOW sfi = { 0 };

    // On demande au système les informations pour un dossier générique.
    // Le drapeau SHGFI_SYSICONINDEX est la clé : il demande le handle de la liste globale.
    // Le drapeau SHGFI_SMALLICON spécifie qu'on veut les icônes de taille 16x16 pixels.
    hSystemImageList = (HIMAGELIST)SHGetFileInfoW(
        L"C:\\",
        FILE_ATTRIBUTE_DIRECTORY,
        &sfi,
        sizeof(sfi),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
    );
}

static inline void ChangeSortingMethod(ColumnListView clickedColumn) {
    if (g_sortColumn == clickedColumn)
    {
        g_TriCroissant = !g_TriCroissant;
    }
    else
    {
        g_sortColumn = clickedColumn;
        g_TriCroissant = TRUE;
    }
}

static inline int CompareByDirectory(ElementFile* A, ElementFile* B) {
    BOOL isDirA = !!(A->flags & ElementDirectory);
    BOOL isDirB = !!(B->flags & ElementDirectory);

    return isDirB - isDirA;
}

static inline void SortAt(ColumnListView clickedColumn) {
    switch (clickedColumn) {
    case COLUMN_LISTVIEW_NAME:
        qsort(m_list->file, m_list->count, sizeof(ElementFile), CompareByName);
        break;
    case COLUMN_LISTVIEW_SIZE:
        qsort(m_list->file, m_list->count, sizeof(ElementFile), CompareBySize);
        break;
    }
}
