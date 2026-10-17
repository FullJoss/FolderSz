/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include "dialog.h"
#include "backend.h"
#include "dpi.h"

#define CLAMP(val, vmin, vmax) ( ((val) < (vmin)) ? (vmin) : (((val) > (vmax)) ? (vmax) : (val)) )
#define ROW_GAP             2
#define ICON_PADDING        6
#define TEXT_PADDING        10
#define SCROLLBAR_WIDTH     14
#define COL_HEADER_HEIGHT   34
#define MIN_THUMB_HEIGHT    20

static wchar_t sizeBuffer[32];
static int ROW_HEIGHT = 26;
static int ICON_SIZE = 16;
static int ARROW_WIDTH = 10;
static int ARROW_HEIGHT = 6;
static int ARROW_MARGE_RIGHT = 10;

static inline void DrawHeaderTitle(HDC hdc, ColumnListView* col, int nb_col, RECT bounds, int indexHover, int indexSort, BOOL isAscending);
static inline void DrawScrollBar(HDC hdc, ListFileView* __restrict lfv, int viewHeight, int totalContentHeight);
static inline void DrawItem(HDC hdc, void* __restrict row, RECT rowBounds, RECT itemsArea, ColumnListView* col, int n_col);
static inline void DrawColumnImageList(HDC hdc, int indexIcon, int paddingLeft, RECT rowBounds, RECT itemsArea);
static inline void DrawColumnText(HDC hdc, const wchar_t* __restrict text, RECT textBounds);
static inline RECT GetNextRowBounds(int i, int scrollPos, const RECT area);
static inline RECT GetNextTextBounds(int x, int row, RECT rowBounds, RECT itemsArea);
static inline int GetHoverIndex(HWND hWnd, int ymouse, ListFileView* lfv);
static inline void MoveHorizontalRect(RECT* r, const RECT limit, int width, int value);
static inline void InvalidatePairItem(HWND hwnd, RECT bounds, int scrollPos, int i1, int i2);
static inline POINT GetIndexRange(ListFileView* __restrict lfv, RECT rcPaint);
static inline void CalculateClipRegion(ListFileView* lfv);
static inline void CalculateItemsArea(ListFileView* lfv);
static inline void CalculateTrackThumb(ListFileView* lfv);
static inline RECT GetThumbRect(ListFileView* lfv);
static inline void SortListFileView(ListFileView* lfv);
static inline int GetNewScrollPos(int ymouse, ListFileView* lfv);
static void CustomSort(void* tab, size_t nb, size_t sizeElem, CallbackSort funcSort, BOOL isAscending);
static inline int FindIndexHeaderTitle(int x, int initial, ColumnListView* col, int nCol);
void DrawSortArrow(HDC hdc, RECT rcColumn, BOOL isAscending);














void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint) {

    FillRect(hdc, &lfv->bounds, g_brushs[IC_BTN_BACKGROUND]);

    SetBkMode(hdc, TRANSPARENT);

    void *rows = lfv->rows;
    if (rows && lfv->n_rows > 0)
    {
        const RECT itemsArea = lfv->itemsArea;

        int totalContentHeight = lfv->n_rows * ROW_HEIGHT;
        int viewHeight = RectHeight(&itemsArea);

        RECT rowBounds = { 0 };

        POINT i_range = GetIndexRange(lfv, rcPaint);

        SelectClipRgn(hdc, lfv->clipRgn);
        
        for (int i = i_range.x; i < i_range.y; i++) {

            rowBounds = GetNextRowBounds(i, lfv->scrollPos, itemsArea);

            void* row = ((char*)rows) + (i * lfv->row_size);

            if (rowBounds.bottom < itemsArea.top) continue;
            if (rowBounds.top > itemsArea.bottom) break;

            if (i == lfv->indexRowHover)
                FillRect(hdc, &rowBounds, g_brushs[IC_BTN_HOVER]);

            DrawItem(hdc, row, rowBounds, itemsArea, lfv->columns, lfv->n_columns);
        }
        
        SelectClipRgn(hdc, NULL);

        if (totalContentHeight > viewHeight) {
            DrawScrollBar(hdc, lfv, viewHeight, totalContentHeight);
        }
    }

    FrameRect(hdc, &lfv->bounds, g_brushs[IC_BTN_BORDER]);

    DrawHeaderTitle(hdc, lfv->columns, lfv->n_columns, lfv->bounds, lfv->indexColHover, lfv->indexColSort, lfv->ascendingSort);
}


void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* lfv) {
   
    POINT pt = { x, y };

    BOOL headerClick = PtInRect(&lfv->headRect, pt);
    if (headerClick) {
        int i = FindIndexHeaderTitle(x, lfv->headRect.left, lfv->columns, lfv->n_columns);
        
        lfv->indexColSort = i;

        SortListFileView(lfv);

        InvalidateRect(hWnd, &lfv->bounds, FALSE);
    }

    if (!lfv->rows || !lfv->enableScroll)
        return;

    BOOL trackClicked = PtInRect(&lfv->sbTrack, pt);
    
    // Thumb is in the Track
    if (!trackClicked)
        return;

    RECT thumb = GetThumbRect(lfv);

    lfv->isDraggingScroll = TRUE;
    if (PtInRect(&thumb, pt)) {
        lfv->dragYOffset = y - thumb.top;

        // Drag out window
        SetCapture(hWnd);
    }
    else 
        lfv->scrollPos = GetNewScrollPos(pt.y, lfv);

    InvalidateRect(hWnd, &lfv->bounds, FALSE);
}

static inline int FindIndexHeaderTitle(int x, int initial, ColumnListView* col, int nCol) {
    int xtest = initial;
    for (int i = 0; i < nCol; i++) {
        if (x >= xtest && x <= xtest + col[i].width) {
            return i;
        }
        xtest += col[i].width;
    }
    return -1;
}

void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* lfv) {

    int newIHover = GetHoverIndex(hWnd, y, lfv);
    if (newIHover != lfv->indexRowHover)
    {
        InvalidatePairItem(hWnd, lfv->itemsArea, lfv->scrollPos, newIHover, lfv->indexRowHover);
        lfv->indexRowHover = newIHover;
    }

    RECT headRect = lfv->headRect;
    POINT mouse = { x, y };
    int i_col = -1;
    if (PtInRect(&headRect, mouse))
    {
        i_col = FindIndexHeaderTitle(x, headRect.left, lfv->columns, lfv->n_columns);
    }

    if (i_col != lfv->indexColHover)
    {
        lfv->indexColHover = i_col;
        InvalidateRect(hWnd, &headRect, FALSE);
    }
    
    if (!lfv->isDraggingScroll || !lfv->enableScroll) return;

    int newScrollPos = GetNewScrollPos(y, lfv);

    if (newScrollPos != lfv->scrollPos) {
        lfv->scrollPos = newScrollPos;
        InvalidateRect(hWnd, &lfv->bounds, FALSE);
    }
}


void HandleListViewLButtonUp(HWND hWnd, ListFileView* listView) {
    if (listView->isDraggingScroll) {
        listView->isDraggingScroll = FALSE;
        ReleaseCapture();
        InvalidateRect(hWnd, &listView->bounds, FALSE);
    }
}


void HandleListViewMouseWheel(HWND hWnd, int wheelDelta, LPARAM lParam, ListFileView* lfv) {
    if (!lfv || !lfv->enableScroll) return;
    
    POINT mousePt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    ScreenToClient(hWnd, &mousePt);

    if (!PtInRect(&lfv->bounds, mousePt)) {
        return;
    }

    int scrollCrans = wheelDelta / WHEEL_DELTA;

    int linesToScroll = 3;
    int scrollSensitivity = linesToScroll * ROW_HEIGHT;
    int pixelScrollAmount = scrollCrans * scrollSensitivity;

    int newScrollPos = lfv->scrollPos - pixelScrollAmount;

    newScrollPos = CLAMP(newScrollPos, 0, lfv->maxScrollPos);

    if (newScrollPos != lfv->scrollPos) {
        lfv->scrollPos = newScrollPos;
        InvalidateRect(hWnd, &lfv->bounds, FALSE);
    }
}

















void UpdateListFileView(HWND hwnd, ListFileView* lfv, ListFile* newlist, BOOL sort)
{
    // DPI
    ROW_HEIGHT = DPI_CONST(26);
    ICON_SIZE = DPI_CONST(16);
    ARROW_WIDTH = DPI_CONST(10);
    ARROW_HEIGHT = DPI_CONST(6);
    ARROW_MARGE_RIGHT = DPI_CONST(10);

    lfv->indexColHover = -1;
    lfv->indexRowHover = -1;

    RECT headRect = lfv->bounds;
    headRect.bottom = lfv->bounds.top + COL_HEADER_HEIGHT;
    lfv->headRect = headRect;

    int nb_elem = newlist->count;

    if (newlist && nb_elem > 0) {

        lfv->rows = newlist->files;

        lfv->n_rows = nb_elem;


        CalculateItemsArea(lfv);

        CalculateTrackThumb(lfv);

        if (sort) SortListFileView(lfv);
        else {
            lfv->scrollPos = lfv->maxScrollPos;
        }

        CalculateClipRegion(lfv);
    }

    InvalidateRect(hwnd, &lfv->bounds, FALSE);
    UpdateWindow(hwnd);
}

void FreeListFileView(ListFileView* lfv) {
    if (lfv->clipRgn) DeleteObject(lfv->clipRgn);
    lfv->rows = NULL;
    lfv->n_rows = 0;
    lfv->row_size = 0;
    lfv->scrollPos = 0;
}












static inline void DrawHeaderTitle(HDC hdc, ColumnListView* col, int nb_col, RECT bounds, int indexHover, int indexSort, BOOL isAscending) {
    
    if (!col || nb_col == 0) return;
    
    RECT headerZone = bounds;
    headerZone.right -= SCROLLBAR_WIDTH;
    headerZone.bottom = bounds.top + COL_HEADER_HEIGHT;

    SetTextColor(hdc, g_colors[IC_TEXT]);

    const int padding_left = 20;
    int lastX = headerZone.left;

    RECT titleZone = headerZone;
    RECT textZone = headerZone;

    for (int i = 0; i < nb_col; i++) {

        MoveHorizontalRect(&titleZone, bounds, col[i].width, lastX);
        MoveHorizontalRect(&textZone , bounds, col[i].width, lastX + padding_left);

        if (i == indexHover)
        {
            FillRect(hdc, &titleZone, g_brushs[IC_BTN_HOVER]);
            FrameRect(hdc, &titleZone, g_brushs[IC_BTN_HOVER_BORDER]);
        }
        else {
            FrameRect(hdc, &titleZone, g_brushs[IC_BTN_BORDER]);
        }

        if (i == indexSort) {
            DrawSortArrow(hdc, titleZone, isAscending);
        }
        
        DrawTextW(hdc, col[i].name, -1, &textZone, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        lastX += col[i].width;
    }
}

static inline void DrawScrollBar(HDC hdc, ListFileView* __restrict lfv, int viewHeight, int totalContentHeight) {
    FillRect(hdc, &lfv->sbTrack, g_brushs[IC_BTN_HOVER]);

    ColorIndex iColor = lfv->isDraggingScroll ? IC_BTN_BORDER : IC_BTN_CLICK;
    
    RECT sbThumb = GetThumbRect(lfv);
    FillRect(hdc, &sbThumb, g_brushs[iColor]);
}

static inline const wchar_t* GetRowTextElementFile(ElementFile* file, int i_col) {
    switch (i_col) {
    case 0:
        return file->name;
    case 1:
        if (file->flags & ElementSystem)
            return L"System File";
        StrFormatByteSizeW(file->size, sizeBuffer, ARRAYSIZE(sizeBuffer));
        return sizeBuffer;
    default:
        return L"-";
    }
}

static inline void DrawItem(HDC hdc, void* __restrict row, RECT rowBounds, RECT itemsArea, ColumnListView* col, int n_col) {
    ElementFile* file = (ElementFile*)row;
    DrawColumnImageList(hdc, file->indexIcon, DPI_CONST(5), rowBounds, itemsArea);

    RECT textBounds;

    const int paddingLeft = DPI_CONST(8 + 5);
    int x = paddingLeft + ICON_SIZE;
    const int gap = 16;
    for (int i = 0; i < n_col; i++) {

        int width = col[i].width;
        if (i == 0) width -= paddingLeft + ICON_SIZE;

        textBounds = GetNextTextBounds(x, width, rowBounds, itemsArea);
        DrawColumnText(hdc, GetRowTextElementFile(file, i), textBounds);

        x += width + gap;
    }
}

static inline void DrawColumnImageList(HDC hdc, int indexIcon, int paddingLeft, RECT rowBounds, RECT itemsArea) {
    int iconY = rowBounds.top + ((ROW_HEIGHT - ICON_SIZE) / 2);
    ImageList_DrawEx(
        g_systemImageList,
        indexIcon,
        hdc,
        itemsArea.left + paddingLeft,
        iconY,
        0, 0,                        // Native Size
        CLR_NONE,                    // Transparent
        CLR_NONE,                    // Transparent
        ILD_NORMAL | ILD_SCALE      
    );
}

static inline void DrawColumnText(HDC hdc, const wchar_t* __restrict text, RECT textBounds) {
    SetTextColor(hdc, g_colors[IC_TEXT]);
    int decalage = (RectHeight(&textBounds) - g_fontHeight) / 2;
    ExtTextOutW(hdc, textBounds.left, textBounds.top + decalage,
        ETO_CLIPPED,
        &textBounds,
        text,
        wcslen(text),
        NULL);
}

static inline RECT GetNextRowBounds(int i, int scrollPos, const RECT area) {
    RECT rowBounds;
    rowBounds.left = area.left;
    rowBounds.right = area.right;
    rowBounds.top = area.top + (i * (ROW_HEIGHT + ROW_GAP)) - scrollPos;
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

static inline int GetHoverIndex(HWND hWnd, int ymouse, ListFileView* lfv) {
    int localY = ymouse - lfv->itemsArea.top;

    if (localY < 0)
        return -1;

    int virtualY = localY + lfv->scrollPos;

    int newhoverI = virtualY / (ROW_HEIGHT + ROW_GAP);

    return newhoverI;
}

static inline int GetNewScrollPos(int ymouse, ListFileView* lfv) {

    int maxScrollPos = lfv->maxScrollPos;

    int maxThumbY =  lfv->maxThumbY;

    int proposedThumbTop = ymouse - lfv->dragYOffset - lfv->itemsArea.top;
    proposedThumbTop = CLAMP(proposedThumbTop, 0, maxThumbY);

    /*
        maxScrollPos        ==>      maxThumbY
        newScrollPos ??     <==      proposedThumbTop
    */
    return (proposedThumbTop * maxScrollPos) / maxThumbY;
}

static inline void MoveHorizontalRect(RECT* r, const RECT limit, int width, int value) {
    r->left = value;
    r->right = value + width;

    r->left = max(r->left, limit.left);
    r->right = min(r->right, limit.right);
}

static inline void InvalidatePairItem(HWND hwnd, RECT area, int scrollPos, int i1, int i2) {
    RECT lastR = area;
    lastR.top += (i1 * (ROW_HEIGHT + ROW_GAP)) - scrollPos;
    lastR.bottom = lastR.top + ROW_HEIGHT;

    RECT newR = area;
    newR.top += (i2 * (ROW_HEIGHT + ROW_GAP)) - scrollPos;
    newR.bottom = newR.top + ROW_HEIGHT;

    InvalidateRect(hwnd, &lastR, FALSE);
    InvalidateRect(hwnd, &newR, FALSE);
}

static inline POINT GetIndexRange(ListFileView* __restrict lfv, RECT rcPaint) {
    int paintTopVirtual = (rcPaint.top - lfv->itemsArea.top) + lfv->scrollPos;
    int paintBottomVirtual = (rcPaint.bottom - lfv->itemsArea.top) + lfv->scrollPos;

    int fullRowHeight = ROW_HEIGHT + ROW_GAP;

    int startIndex = paintTopVirtual / fullRowHeight;

    // integer division rounding up
    unsigned int endIndex = (paintBottomVirtual + fullRowHeight - 1) / fullRowHeight;

    if (startIndex < 0) startIndex = 0;
    if (endIndex >= lfv->n_rows) endIndex = lfv->n_rows - 1;

    return (POINT){ startIndex, endIndex };
}

static inline void CalculateClipRegion(ListFileView* lfv) {
    if (lfv->clipRgn)
    {
        RECT clipRect;
        GetRgnBox(lfv->clipRgn, &clipRect);

        if (EqualRect(&lfv->bounds, &clipRect)) 
            return;
    }

    lfv->clipRgn = CreateRectRgnIndirect(&lfv->itemsArea);
}

static inline void CalculateItemsArea(ListFileView* lfv) {
    lfv->itemsArea = lfv->bounds;
    lfv->itemsArea.top += COL_HEADER_HEIGHT;
    lfv->itemsArea.right -= SCROLLBAR_WIDTH;
}

static inline void CalculateTrackThumb(ListFileView* lfv) {
    RECT itemsArea = lfv->itemsArea;
    RECT bounds = lfv->bounds;
    int dataCount = lfv->n_rows;

    lfv->sbTrack = (RECT){ itemsArea.right, itemsArea.top, bounds.right, itemsArea.bottom };

    int totalContentHeightExpect = (dataCount - 1) * ROW_HEIGHT;
    int totalGapHeight = dataCount * ROW_GAP;
    int totalContentHeight = totalContentHeightExpect + totalGapHeight;
    int viewHeight = RectHeight(&itemsArea);

    int thumbHeight = (viewHeight * viewHeight) / totalContentHeight;

    lfv->sbThumbHeight = max(thumbHeight, MIN_THUMB_HEIGHT);

    int maxScrollPos = totalContentHeight - viewHeight;
    lfv->maxScrollPos = max(0, maxScrollPos);

    lfv->maxThumbY = viewHeight - lfv->sbThumbHeight;

    lfv->enableScroll = totalContentHeight > viewHeight;
}


static inline RECT GetThumbRect(ListFileView* lfv) {
    RECT track = lfv->sbTrack;
    
    int maxScrollPos = lfv->maxScrollPos;
    int maxThumbY = lfv->maxThumbY;

    int thumbTop = track.top + (lfv->scrollPos * maxThumbY) / maxScrollPos;

    return (RECT) { track.left, thumbTop, track.right, thumbTop + lfv->sbThumbHeight };
}

static inline void SortListFileView(ListFileView* lfv) {
    lfv->scrollPos = 0;
    ColumnListView* col = lfv->columns + lfv->indexColSort;
    CustomSort(lfv->rows, lfv->n_rows, lfv->row_size, col->FuncComparison, lfv->ascendingSort);

    lfv->ascendingSort = !lfv->ascendingSort;
}

static void CustomSort(void* tab, size_t nb, size_t sizeElem, CallbackSort funcSort, BOOL isAscending) {
    
    void* temp = malloc(sizeElem);
    if (!temp) return;
    
    for (size_t i = 0; i < nb - 1; i++) {

        void* tab_i = (char*)tab + i * sizeElem;

        size_t targetI = i;
        for (size_t j = i + 1; j < nb; j++) {

            void* tab_tI = (char*)tab + targetI * sizeElem;
            void* tab_j = (char*)tab + j * sizeElem;

            int valueComparison = funcSort(tab_j, tab_tI, isAscending);

            if (valueComparison < 0) {
                targetI = j;
            }
        }
        
        if (targetI != i) {
            void* tab_target = (char*)tab + targetI * sizeElem;

            memcpy(temp, tab_i, sizeElem);
            memcpy(tab_i, tab_target, sizeElem);
            memcpy(tab_target, temp, sizeElem);
        }
    }

    free(temp);
}

// ============================================================================
// RESPONSABILITÉ : Dessine une petite flèche de tri (triangle plein) en GDI
// ============================================================================
void DrawSortArrow(HDC hdc, RECT rcColumn, BOOL isAscending) {

    int centerY = rcColumn.top + ((rcColumn.bottom - rcColumn.top) - ARROW_HEIGHT) / 2;
    int arrowRight = rcColumn.right - ARROW_MARGE_RIGHT;
    int arrowLeft = arrowRight - ARROW_WIDTH;

    POINT points[3];
    if (isAscending) {
        points[0].x = arrowLeft + (ARROW_WIDTH / 2); points[0].y = centerY;                 // Sommet
        points[1].x = arrowLeft;                    points[1].y = centerY + ARROW_HEIGHT;   // Bas Gauche
        points[2].x = arrowRight;                   points[2].y = centerY + ARROW_HEIGHT;   // Bas Droite
    }
    else {
        points[0].x = arrowLeft;                    points[0].y = centerY;                 // Haut Gauche
        points[1].x = arrowRight;                   points[1].y = centerY;                 // Haut Droite
        points[2].x = arrowLeft + (ARROW_WIDTH / 2); points[2].y = centerY + ARROW_HEIGHT;   // Pointe Bas
    }

    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_brushs[IC_BTN_HOVER_BORDER]);
    HPEN hOldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));

    Polygon(hdc, points, 3);

    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
}
