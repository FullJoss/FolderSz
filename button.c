#include "button.h"
#include "theme.h"

static inline void DrawBorder(HDC hdc, Button* button);
static inline void WriteText(HDC hdc, Button* button);
static inline void DrawBackground(HDC hdc, Button* button);
static inline int GetButtonState(WPARAM wParam, BOOL isOver);
static inline ColorIndex ButonStateToIndexColor(int state);

Button CreateButton(const wchar_t* name, Callback event) {
    Button btn = { 0 };
    btn.text = name;
    btn.state = PBS_NORMAL;
    btn.on_click = event;
    return btn;
}

BOOL ButtonHitTest(WPARAM wParam, int x, int y, Button *button)
{
    BOOL isOver = PtInRect(&button->bounds, (POINT){x, y});
    int newstate = GetButtonState(wParam, isOver);

    if (newstate != button->state)
    {
        button->state = newstate;
        InvalidateRect(mainHwnd, &button->bounds, FALSE);
    }

    return isOver;
}

BOOL ButtonClickTest(WPARAM wParam, int x, int y, Button* button, WindowDataInformation* pData)
{
    if (wParam & MK_LBUTTON)
    {
        POINT p = {x, y};

        if (PtInRect(&button->bounds, p))
        {
            button->on_click(pData);
            return TRUE;
        }
    }
    return FALSE;
}

void ButtonDraw(HDC hdc, Button *button)
{
    DrawBackground(hdc, button);
    DrawBorder(hdc, button);
    WriteText(hdc, button);
}

static inline void WriteText(HDC hdc, Button* button) {
    SetTextColor(hdc, g_colors[IC_TEXT]);
    SetBkMode(hdc, TRANSPARENT);

    DrawTextW(hdc, button->text, -1, (RECT*)&button->bounds, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static inline void DrawBorder(HDC hdc, Button* button) {
    int index = (button->state == PBS_HOT) ? IC_BTN_HOVER_BORDER : IC_BTN_BORDER;
    FrameRect(hdc, &button->bounds, g_brushs[index]);
}

static inline void DrawBackground(HDC hdc, Button* button) {
    ColorIndex index = ButonStateToIndexColor(button->state);
    FillRect(hdc, &button->bounds, g_brushs[index]);
}

static inline int GetButtonState(WPARAM wParam, BOOL isOver) {
    int newstate = isOver ? PBS_HOT : PBS_NORMAL;

    BOOL isLeftPressed = (wParam & MK_LBUTTON);
    if (isLeftPressed)
    {
        newstate = isOver ? PBS_PRESSED : PBS_NORMAL;
    }

    return newstate;
}

static inline ColorIndex ButonStateToIndexColor(int state) {
    switch (state) {
    case PBS_PRESSED:
        return IC_BTN_CLICK;
    case PBS_HOT:
        return IC_BTN_HOVER;
    default:
        return IC_BTN_BACKGROUND;
    }
}