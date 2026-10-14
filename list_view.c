#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include "dialog.h"

#define ROW_HEIGHT          26
#define COLUMN_0_WIDTH      300
#define ICON_SIZE           16
#define ICON_PADDING        6
#define TEXT_PADDING        10
#define SCROLLBAR_WIDTH     14
#define VERT_ITEM_SPACE     2

BOOL g_TriCroissant = TRUE;
ColumnListView g_sortColumn = COLUMN_LISTVIEW_NAME;

static wchar_t sizeBuffer[32];
static HWND m_hwnd = NULL;
static ListFile* m_list;


static inline void DrawItem(HDC hdc, ElementFile* __restrict file, RECT rowBounds, RECT itemsArea);
static inline void DrawColumnImageList(HDC hdc, int indexIcon, int paddingLeft, RECT rowBounds, RECT itemsArea);
static inline void DrawColumnText(HDC hdc, const wchar_t* __restrict text, RECT textBounds);
static inline RECT GetNextRowBounds(int i, int scrollPos, RECT area);
static inline RECT GetNextTextBounds(int x, int row, RECT rowBounds, RECT itemsArea);
static int CompareByName(const void* a, const void* b);
static int CompareBySize(const void* a, const void* b);
static inline void ChangeSortingMethod(ColumnListView clickedColumn);
static inline int CompareByDirectory(ElementFile* A, ElementFile* B);
static inline void SortAt(ColumnListView clickedColumn);

RECT GetScrollThumbRect(ListFileView* listView);


















void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint) {
    ListFile *list = lfv->data;
    if (!list) return;

    SelectClipRgn(hdc, lfv->clipRgn);

    FillRect(hdc, &lfv->bounds, g_brushs[IC_BTN_BACKGROUND]);

    int paintTopVirtual = (rcPaint.top - lfv->bounds.top) + lfv->scrollPos;
    int paintBottomVirtual = (rcPaint.bottom - lfv->bounds.top) + lfv->scrollPos;

    // Traduction directe en index de tableau (sans boucle, par division)
    int fullRowHeight = ROW_HEIGHT + 2;
    int startIndex = paintTopVirtual / fullRowHeight;
    int endIndex = (paintBottomVirtual + fullRowHeight - 1) / fullRowHeight;

    RECT itemsArea = lfv->bounds;
    itemsArea.right -= SCROLLBAR_WIDTH;

    if (list->count <= 0 || !list->file)
        return;

    int totalContentHeight = list->count * ROW_HEIGHT;
    int viewHeight = itemsArea.bottom - itemsArea.top;

    SetBkMode(hdc, TRANSPARENT);

    RECT rowBounds = { 0 };

    if (startIndex < 0) startIndex = 0;
    if (endIndex >= list->count) endIndex = list->count - 1;

    for (size_t i = startIndex; i < endIndex; i++) {
        rowBounds = GetNextRowBounds(i, lfv->scrollPos, itemsArea);

        ElementFile* file = list->file + i;

        if (rowBounds.bottom < itemsArea.top) continue;
        if (rowBounds.top > itemsArea.bottom) break;

        if (i == lfv->indexHover)
            FillRect(hdc, &rowBounds, g_brushs[IC_BTN_HOVER]);

        DrawItem(hdc, file, rowBounds, itemsArea);
    }

    // 4. DESSIN DE LA SCROLLBAR SUR MESURE (Rectangle à droite)
    if (totalContentHeight > viewHeight) { // On ne la dessine que si ça déborde
        RECT sbTrack = { itemsArea.right, lfv->bounds.top, lfv->bounds.right, lfv->bounds.bottom };

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

    int localY = y - listView->bounds.top;
    int virtualY = localY + listView->scrollPos;

    int newhoverI = virtualY / (ROW_HEIGHT + VERT_ITEM_SPACE);

    if (newhoverI != listView->indexHover) {
        RECT lastR = listView->bounds;
        lastR.top += (listView->indexHover * (ROW_HEIGHT + VERT_ITEM_SPACE)) - listView->scrollPos;
        lastR.bottom = lastR.top + ROW_HEIGHT;

        RECT newR = listView->bounds;
        newR.top += (newhoverI * (ROW_HEIGHT + VERT_ITEM_SPACE)) - listView->scrollPos;
        newR.bottom = newR.top + ROW_HEIGHT;

        listView->indexHover = newhoverI;
        InvalidateRect(hWnd, &lastR, FALSE);
        InvalidateRect(hWnd, &newR, FALSE);
        //InvalidateRect(hWnd, &listView->bounds, FALSE);
    }

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

void DeleteListFileView(ListFileView* lfv) {
    DeleteObject(lfv->clipRgn);
}



















static inline void DrawItem(HDC hdc, ElementFile* __restrict file, RECT rowBounds, RECT itemsArea) {
    DrawColumnImageList(hdc, file->indexIcon, 5, rowBounds, itemsArea);

    RECT textBounds;

    int x_column_0 = 8 + 5 + ICON_SIZE;
    textBounds = GetNextTextBounds(x_column_0, COLUMN_0_WIDTH, rowBounds, itemsArea);

    DrawColumnText(hdc, file->name, textBounds);

    int x_column_1 = x_column_0 + COLUMN_0_WIDTH + 5;
    textBounds = GetNextTextBounds(x_column_1, COLUMN_0_WIDTH, rowBounds, itemsArea);

    StrFormatByteSizeW(file->size, sizeBuffer, ARRAYSIZE(sizeBuffer));
    DrawColumnText(hdc, sizeBuffer, textBounds);
}

static inline void DrawColumnImageList(HDC hdc, int indexIcon, int paddingLeft, RECT rowBounds, RECT itemsArea) {
    int iconY = rowBounds.top + ((ROW_HEIGHT - ICON_SIZE) / 2);
    ImageList_Draw(g_systemImageList, indexIcon, hdc, itemsArea.left + paddingLeft, iconY, ILD_TRANSPARENT);
}

static inline void DrawColumnText(HDC hdc, const wchar_t* __restrict text, RECT textBounds) {
    SetTextColor(hdc, g_colors[IC_TEXT]);
    DrawTextW(hdc, text, -1, &textBounds, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static inline RECT GetNextRowBounds(int i, int scrollPos, RECT area) {
    RECT rowBounds;
    rowBounds.left = area.left;
    rowBounds.right = area.right;
    rowBounds.top = area.top + (i * (ROW_HEIGHT + VERT_ITEM_SPACE)) - scrollPos;
    rowBounds.bottom = rowBounds.top + ROW_HEIGHT;
    return rowBounds;
}

static inline RECT GetNextTextBounds(int x, int width, RECT rowBounds, RECT itemsArea) {
    RECT textBounds;
    textBounds.left = itemsArea.left + x;
    textBounds.right = itemsArea.left + x + width;
    textBounds.top = rowBounds.top;
    textBounds.bottom = rowBounds.bottom;
    return textBounds;
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
