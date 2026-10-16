#include "layout.h"

#define PADDING 10
#define GAP 10

RECT g_rects[RECT_LAYOUT_COUNT] = { 0 };

int g_CursorX = PADDING;
int g_CursorY = PADDING;
int g_CurrentLineHeight = PADDING;

static void Fill(RectLayout rectTarget, int width, int height);
static inline void EndLine();

void InitLayout(int windowWidth, int windowHeight) {
    
    Fill(RECT_LAYOUT_BUTTON_OPEN_FOLDER, 200, 38);
    Fill(RECT_LAYOUT_BUTTON_SYNC_EXPLR, 200, 38);
    
    EndLine();

    Fill(RECT_LAYOUT_LIST_VIEW, 400, 600);
}

int GetWThird(RectLayout rectTarget) {
    int list_width = g_rects[rectTarget].right - g_rects[rectTarget].left;
    return list_width / 3;
}

int GetWFillOfThird(RectLayout rectTarget) {
    int list_width = g_rects[rectTarget].right - g_rects[rectTarget].left;
    return list_width - GetWThird(rectTarget);
}

static void Fill(RectLayout rectTarget, int width, int height) {
    
    g_rects[rectTarget].left = g_CursorX;
    g_rects[rectTarget].top = g_CursorY;
    g_rects[rectTarget].right = g_CursorX + width;
    g_rects[rectTarget].bottom = g_CursorY + height;

    g_CursorX += GAP + width;
    g_CurrentLineHeight = max(g_CurrentLineHeight, height);
}

static inline void EndLine() {
    g_CursorX = PADDING;
    g_CursorY = PADDING + g_CursorY + g_CurrentLineHeight;
    g_CurrentLineHeight = 0;
}