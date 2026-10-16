#include "layout.h"

typedef struct {
    int width;
    int height;
    int line;
} Buffer;

#define PADDING 20
#define GAP 20

Buffer g_buffers[RECT_LAYOUT_COUNT];
int g_CurrentLineBuffer = 0;

RECT g_rects[RECT_LAYOUT_COUNT] = { 0 };

int g_CursorX = PADDING;
int g_CursorY = PADDING;
int g_CurrentLineHeight = PADDING;

int m_windowWidth = 0;
int m_windowHeight = 0;

static void Fill(RectLayout rectTarget, int width, int height);
static inline void EndLine();
static void DefineBegin(int wWindow, int hWindow);
static void Define(RectLayout rectTarget, int width, int height);
static void DefineComplete();
static inline void NextLine();
static inline int CalculateElementWidth(int elementCount);
static void ApplyLineLayout(int indexFin, int elementCount, int widthFinal);

void InitLayout(int windowWidth, int windowHeight) {
    
    DefineBegin(windowWidth, windowHeight);

    Define(RECT_LAYOUT_BUTTON_OPEN_FOLDER, -1, 38);
    Define(RECT_LAYOUT_BUTTON_SYNC_EXPLR, -1, 38);
    
    NextLine();

    Define(RECT_LAYOUT_LIST_VIEW, -1, 500);

    DefineComplete();
}

int GetWThird(RectLayout rectTarget) {
    int list_width = g_rects[rectTarget].right - g_rects[rectTarget].left;
    return list_width / 3;
}

int GetWFillOfThird(RectLayout rectTarget) {
    int list_width = g_rects[rectTarget].right - g_rects[rectTarget].left;
    return list_width - GetWThird(rectTarget);
}

static void DefineBegin(int wWindow, int hWindow) {
    m_windowWidth = wWindow;
    m_windowHeight = wWindow;
}

static inline int CalculateElementWidth(int elementCount) {
    if (elementCount <= 0) return 0;

    int widthSpace = m_windowWidth - (PADDING * 2);

    int widthGapTotal = (elementCount > 1) ? GAP * (elementCount - 1) : 0;

    return (widthSpace - widthGapTotal) / elementCount;
}

static void ApplyLineLayout(int indexFin, int elementCount, int widthFinal) {
    int startIndex = (indexFin + 1) - elementCount;

    for (int j = startIndex; j <= indexFin; j++) {
        g_buffers[j].width = widthFinal;
        Fill(j, widthFinal, g_buffers[j].height);
    }
}

static void DefineComplete() {
    int localLine = 0;
    int elementCount = 0;

    for (int i = 0; i < RECT_LAYOUT_COUNT; i++) {
        elementCount++;

        BOOL lastElement = (i == RECT_LAYOUT_COUNT - 1);
        BOOL lineWillChange = lastElement || (g_buffers[i + 1].line != localLine);

        if (lineWillChange) {
            int widthFinal = CalculateElementWidth(elementCount);

            ApplyLineLayout(i, elementCount, widthFinal);

            EndLine();
            elementCount = 0;

            if (!lastElement) {
                localLine = g_buffers[i + 1].line;
            }
        }
    }
}

static void Define(RectLayout rectTarget, int width, int height) {
    g_buffers[rectTarget].width = width;
    g_buffers[rectTarget].height = height;
    g_buffers[rectTarget].line = g_CurrentLineBuffer;
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

static inline void NextLine() {
    g_CurrentLineBuffer++;
}