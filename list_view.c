#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include "dialog.h"

#define CLAMP(val, vmin, vmax) ( ((val) < (vmin)) ? (vmin) : (((val) > (vmax)) ? (vmax) : (val)) )
#define ROW_HEIGHT          26
#define COLUMN_0_WIDTH      300
#define ICON_SIZE           16
#define ICON_PADDING        6
#define TEXT_PADDING        10
#define SCROLLBAR_WIDTH     14
#define COL_HEADER_HEIGHT   34
#define VERT_ITEM_SPACE     2
#define MIN_THUMB_HEIGHT    20

static wchar_t sizeBuffer[32];
static HWND m_hwnd = NULL;
static ListFile* m_list;

static inline void DrawHeaderTitle(HDC hdc, ColumnListView* col, int nb_col, RECT bounds);
static inline void DrawScrollBar(HDC hdc, ListFileView* __restrict lfv, int viewHeight, int totalContentHeight);
static inline void DrawItem(HDC hdc, ElementFile* __restrict file, RECT rowBounds, RECT itemsArea);
static inline void DrawColumnImageList(HDC hdc, int indexIcon, int paddingLeft, RECT rowBounds, RECT itemsArea);
static inline void DrawColumnText(HDC hdc, const wchar_t* __restrict text, RECT textBounds);
static inline RECT GetNextRowBounds(int i, int scrollPos, const RECT area);
static inline RECT GetNextTextBounds(int x, int row, RECT rowBounds, RECT itemsArea);
static inline int GetHoverIndex(HWND hWnd, int ymouse, ListFileView* lfv);
static inline void InvalidatePairItem(HWND hwnd, RECT bounds, int scrollPos, int i1, int i2);
static inline POINT GetIndexRange(ListFileView* __restrict lfv, RECT rcPaint);
static inline void CalculateClipRegion(ListFileView* lfv);
static inline void CalculateItemsArea(ListFileView* lfv);
static inline void CalculateTrackThumb(ListFileView* lfv);
static inline RECT GetThumbRect(ListFileView* lfv, int viewHeight, int totalContentHeight);
static inline void SortListFileView(ListFileView* lfv);
static inline void SortAt(ListFile* data, ColumnListView* clickedColumn);
static inline int GetNewScrollPos(int ymouse, ListFileView* lfv);
















void DrawListFileView(HDC hdc, ListFileView* lfv, RECT rcPaint) {
    ListFile *list = lfv->data;

    FillRect(hdc, &lfv->bounds, g_brushs[IC_BTN_BACKGROUND]);

    SetBkMode(hdc, TRANSPARENT);

    if (list && list->count > 0 && list->file)
    {
        const RECT itemsArea = lfv->itemsArea;

        int totalContentHeight = list->count * ROW_HEIGHT;
        int viewHeight = RectHeight(&itemsArea);

        RECT rowBounds = { 0 };

        POINT i_range = GetIndexRange(lfv, rcPaint);

        SelectClipRgn(hdc, lfv->clipRgn);
        
        for (int i = i_range.x; i < i_range.y; i++) {

            rowBounds = GetNextRowBounds(i, lfv->scrollPos, itemsArea);

            ElementFile* file = list->file + i;

            if (rowBounds.bottom < itemsArea.top) continue;
            if (rowBounds.top > itemsArea.bottom) break;

            if (i == lfv->indexHover)
                FillRect(hdc, &rowBounds, g_brushs[IC_BTN_HOVER]);

            DrawItem(hdc, file, rowBounds, itemsArea);
        }
        
        SelectClipRgn(hdc, NULL);

        if (totalContentHeight > viewHeight) {
            DrawScrollBar(hdc, lfv, viewHeight, totalContentHeight);
        }
    }

    DrawHeaderTitle(hdc, lfv->columns, lfv->n_columns, lfv->bounds);

    FrameRect(hdc, &lfv->bounds, g_brushs[IC_BTN_BORDER]);
}


void HandleListViewLButtonDown(HWND hWnd, int x, int y, ListFileView* listView) {
    
    if (!listView->data) return;
    
    POINT pt = { x, y };

    BOOL trackClicked = PtInRect(&listView->sbTrack, pt);
    
    // Thumb is in the Track
    if (!trackClicked)
        return;

    int totalContentHeight = listView->data->count * ROW_HEIGHT;
    int viewHeight = RectHeight(&listView->bounds);

    RECT thumb = GetThumbRect(listView, viewHeight, totalContentHeight);

    listView->isDraggingScroll = TRUE;
    if (PtInRect(&thumb, pt)) {
        listView->dragYOffset = y - thumb.top;

        // Drag out window
        SetCapture(hWnd);
    }
    else 
        listView->scrollPos = GetNewScrollPos(pt.y, listView);

    InvalidateRect(hWnd, &listView->bounds, FALSE);
}

void HandleListViewMouseMove(HWND hWnd, int x, int y, ListFileView* listView) {

    int newIHover = GetHoverIndex(hWnd, y, listView);
    if (newIHover != listView->indexHover)
    {
        InvalidatePairItem(hWnd, listView->itemsArea, listView->scrollPos, newIHover, listView->indexHover);
        listView->indexHover = newIHover;
    }
    
    if (!listView->isDraggingScroll) return;

    int newScrollPos = GetNewScrollPos(y, listView);

    if (newScrollPos != listView->scrollPos) {
        listView->scrollPos = newScrollPos;
        InvalidateRect(hWnd, &listView->bounds, FALSE);
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
    
    POINT mousePt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    ScreenToClient(hWnd, &mousePt);

    if (!PtInRect(&listView->bounds, mousePt)) {
        return;
    }

    int scrollCrans = wheelDelta / WHEEL_DELTA;

    int linesToScroll = 3;
    int scrollSensitivity = linesToScroll * ROW_HEIGHT;
    int pixelScrollAmount = scrollCrans * scrollSensitivity;

    int newScrollPos = listView->scrollPos - pixelScrollAmount;

    int totalContentHeight = listView->data->count * ROW_HEIGHT;
    int viewHeight = RectHeight(&listView->itemsArea);

    int maxScrollPos = max(0, totalContentHeight - viewHeight);

    newScrollPos = CLAMP(newScrollPos, 0, maxScrollPos);

    if (newScrollPos != listView->scrollPos) {
        listView->scrollPos = newScrollPos;
        InvalidateRect(hWnd, &listView->bounds, FALSE); 
    }
}



















void UpdateListFileView(ListFileView* lfv)
{
    lfv->scrollPos = 0;

    if (!lfv->data || !lfv->data->count) return;

    SortListFileView(lfv);
    
    CalculateItemsArea(lfv);

    CalculateTrackThumb(lfv);
    
    CalculateClipRegion(lfv);
    
    InvalidateRect(lfv->hwnd, &lfv->bounds, FALSE);
}

void FreeListFileView(ListFileView* lfv) {
    if (lfv->clipRgn) DeleteObject(lfv->clipRgn);
}














static inline void DrawHeaderTitle(HDC hdc, ColumnListView* col, int nb_col, RECT bounds) {
    
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

        titleZone.left = lastX;
        titleZone.right = lastX + col[i].width;

        titleZone.left = max(titleZone.left, bounds.left);
        titleZone.right = min(titleZone.right, bounds.right);

        FrameRect(hdc, &titleZone, g_brushs[IC_BTN_BORDER]);

        textZone.left = lastX + padding_left;
        textZone.right = lastX + padding_left + col[i].width;

        textZone.left = max(textZone.left, bounds.left);
        textZone.right = min(textZone.right, bounds.right);

        DrawTextW(hdc, col[i].name, -1, &textZone, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        
        lastX += col[i].width;
    }
}

static inline void DrawScrollBar(HDC hdc, ListFileView* __restrict lfv, int viewHeight, int totalContentHeight) {
    FillRect(hdc, &lfv->sbTrack, g_brushs[IC_BTN_HOVER]);

    ColorIndex iColor = lfv->isDraggingScroll ? IC_BTN_BORDER : IC_BTN_CLICK;
    
    RECT sbThumb = GetThumbRect(lfv, viewHeight, totalContentHeight);
    FillRect(hdc, &sbThumb, g_brushs[iColor]);
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

static inline RECT GetNextRowBounds(int i, int scrollPos, const RECT area) {
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

static inline int GetHoverIndex(HWND hWnd, int ymouse, ListFileView* lfv) {
    int localY = ymouse - lfv->itemsArea.top;
    int virtualY = localY + lfv->scrollPos;

    int newhoverI = virtualY / (ROW_HEIGHT + VERT_ITEM_SPACE);

    return newhoverI;
}

static inline int GetNewScrollPos(int ymouse, ListFileView* lfv) {
    int totalContentHeight = lfv->data->count * ROW_HEIGHT;
    int viewHeight = RectHeight(&lfv->itemsArea);
    
    int maxScrollPos = totalContentHeight - viewHeight;

    int thumbHeight = lfv->sbThumbHeight;

    int maxThumbY = viewHeight - thumbHeight;

    int proposedThumbTop = ymouse - lfv->dragYOffset - lfv->itemsArea.top;
    proposedThumbTop = CLAMP(proposedThumbTop, 0, maxThumbY);

    /*
        maxScrollPos        ==>      maxThumbY
        newScrollPos ??     <==      proposedThumbTop
    */
    return (proposedThumbTop * maxScrollPos) / maxThumbY;
}

static inline void InvalidatePairItem(HWND hwnd, RECT area, int scrollPos, int i1, int i2) {
    RECT lastR = area;
    lastR.top += (i1 * (ROW_HEIGHT + VERT_ITEM_SPACE)) - scrollPos;
    lastR.bottom = lastR.top + ROW_HEIGHT;

    RECT newR = area;
    newR.top += (i2 * (ROW_HEIGHT + VERT_ITEM_SPACE)) - scrollPos;
    newR.bottom = newR.top + ROW_HEIGHT;

    InvalidateRect(hwnd, &lastR, FALSE);
    InvalidateRect(hwnd, &newR, FALSE);
}

static inline POINT GetIndexRange(ListFileView* __restrict lfv, RECT rcPaint) {
    int paintTopVirtual = (rcPaint.top - lfv->itemsArea.top) + lfv->scrollPos;
    int paintBottomVirtual = (rcPaint.bottom - lfv->itemsArea.top) + lfv->scrollPos;

    int fullRowHeight = ROW_HEIGHT + VERT_ITEM_SPACE;

    int startIndex = paintTopVirtual / fullRowHeight;

    // integer division rounding up
    unsigned int endIndex = (paintBottomVirtual + fullRowHeight - 1) / fullRowHeight;

    if (startIndex < 0) startIndex = 0;
    if (endIndex >= lfv->data->count) endIndex = lfv->data->count - 1;

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
    int dataCount = lfv->data->count;

    lfv->sbTrack = (RECT){ itemsArea.right, itemsArea.top, bounds.right, itemsArea.bottom };

    int totalContentHeight = dataCount * ROW_HEIGHT;
    int viewHeight = RectHeight(&itemsArea);

    int thumbHeight = (viewHeight * viewHeight) / totalContentHeight;

    lfv->sbThumbHeight = max(thumbHeight, MIN_THUMB_HEIGHT);
}


static inline RECT GetThumbRect(ListFileView* lfv, int viewHeight, int totalContentHeight) {
    RECT track = lfv->sbTrack;
    
    int maxScrollPos = totalContentHeight - viewHeight;
    int maxThumbY = viewHeight - lfv->sbThumbHeight;

    int thumbTop = track.top + (lfv->scrollPos * maxThumbY) / maxScrollPos;

    return (RECT) { track.left, thumbTop, track.right, thumbTop + lfv->sbThumbHeight };
}

static inline void SortListFileView(ListFileView* lfv) {
    BOOL sorted = FALSE;
    for (int i = 0; i < lfv->n_columns; i++) {
        ColumnListView* col = lfv->columns + i;

        if (sorted) {
            col->isSorted = FALSE;
        } else if (col->isSorted) {
            SortAt(lfv->data, col);
        }
    }
}

static inline void SortAt(ListFile *data, ColumnListView* clickedColumn) {
    qsort(data->file, data->count, sizeof(ElementFile), clickedColumn->FuncComparison);
}