#include "layout.h"

typedef struct {
    int width;
    int height;
    int line;
} Buffer;

#define PADDING 10
#define GAP 10

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
static inline int CalculateElementWidth(int endIndex, int elementCount);
static void ApplyLineLayout(int indexFin, int elementCount, int widthFinal);

void InitLayout(int windowWidth, int windowHeight) {
    
    DefineBegin(windowWidth, windowHeight);

    Define(RECT_LAYOUT_BUTTON_OPEN_FOLDER, -1, 34);
    Define(RECT_LAYOUT_BUTTON_SYNC_EXPLR, 260, 34);
    
    NextLine();

    Define(RECT_LAYOUT_LIST_VIEW, -1, windowHeight - PADDING * 2 - GAP - 34);

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

static inline int CalculateElementWidth(int endIndex, int elementCount) {
    if (elementCount <= 0) return 0;

    int widthSpace = m_windowWidth - (PADDING * 2);
    int widthGapTotal = (elementCount > 1) ? GAP * (elementCount - 1) : 0;

    int remainingSpace = widthSpace - widthGapTotal;
    int dynamicElementCount = 0;

    int startIndex = (endIndex + 1) - elementCount;

    for (int j = startIndex; j <= endIndex; j++) {

        if (g_buffers[j].width > 0) 
            remainingSpace -= g_buffers[j].width;
        
        else 
            dynamicElementCount++;
    }

    if (dynamicElementCount > 0 && remainingSpace > 0) {
        return remainingSpace / dynamicElementCount;
    }

    return 0;
}

static void ApplyLineLayout(int indexFin, int elementCount, int widthFinal) {
    int startIndex = (indexFin + 1) - elementCount;

    for (int j = startIndex; j <= indexFin; j++) {

        if (g_buffers[j].width <= 0) {
            g_buffers[j].width = widthFinal;
        }

        Fill(j, g_buffers[j].width, g_buffers[j].height);
    }
}

static void DefineComplete() {
    int localLine = 0;
    int elementCount = 0;

    for (int i = 0; i < RECT_LAYOUT_COUNT; i++) {
        elementCount++;

        BOOL nextValid = i + 1 < RECT_LAYOUT_COUNT;
        const Buffer* nextBuffer = nextValid ? (g_buffers + i + 1) : NULL;

        if (!nextValid || nextBuffer->line != localLine) {
            int widthFinal = CalculateElementWidth(i, elementCount);

            ApplyLineLayout(i, elementCount, widthFinal);

            EndLine();
            elementCount = 0;

            if (nextBuffer) {
                localLine = nextBuffer->line;
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