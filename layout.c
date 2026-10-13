#include "layout.h"

static int accx = 0;
static int accy = 0;
static int margin = 10;
static int lastWidth = 0;
static int lastHeight = 0;

RECT GetNextBounds(int width, int height, BOOL horizontal) {
    if (horizontal) accx += margin + lastWidth;
    else accy += margin + lastHeight;

    lastWidth = width;
    lastHeight = height;

    return (RECT) { accx, accy, accx + width, accy + height };
}

RECT GetFinalBounds(int total, int fixedValue, BOOL horizontal) {
    int width = fixedValue;
    int height = fixedValue;

    if (horizontal) {
        accx += margin + lastWidth;
        int space = total - accx;
        width = space - margin * 2;
    }
    else {
        accy += margin + lastHeight;
        int space = total - accy;
        height = space - margin * 2;
    }

    return (RECT) { accx, accy, accx + width, accy + height };
}

void SetMargin(int value) {
    margin = value;
}

void SetX(int value) {
    accx = value;
}

void SetY(int value) {
    accy = value;
}