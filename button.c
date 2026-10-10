#include "button.h"
#include "theme.h"

BOOL ButtonHitTest(WPARAM wParam, int x, int y, Button *button)
{
    // Est-ce que la souris est dans le rectangle du bouton ?
    BOOL isOver = PtInRect(&button->bounds, (POINT){x, y});
    int nouvelEtat = isOver ? PBS_HOT : PBS_NORMAL;

    // Si l'utilisateur clique et glisse en dehors
    if (wParam & MK_LBUTTON)
    {
        nouvelEtat = isOver ? PBS_PRESSED : PBS_NORMAL;
    }

    // Si l'état a changé, on demande à redessiner la zone du bouton
    if (nouvelEtat != button->state)
    {
        button->state = nouvelEtat;
        InvalidateRect(mainHwnd, &button->bounds, FALSE); // Force le message WM_PAINT
    }

    return isOver;
}

BOOL ButtonClickTest(WPARAM wParam, int x, int y, Button *button)
{
    if (wParam & MK_LBUTTON)
    {
        POINT p = {x, y};
        if (PtInRect(&button->bounds, p))
        {
            button->on_click();

            return TRUE;
        }
    }
    return FALSE;
}

void ButtonDraw(HDC hdc, Button *button)
{
    FillRect(hdc, &button->bounds, get_btn_brush(button->state));

    // 3. Dessiner la bordure fine du bouton
    FrameRect(hdc, &button->bounds, get_btn_brush_border());

    // 4. Dessiner le texte net (avec votre police Segoe UI)
    SetTextColor(hdc, get_btn_color_text());
    SetBkMode(hdc, TRANSPARENT); // Évite un carré blanc derrière les lettres

    DrawTextW(hdc, button->text, -1, (RECT *)&button->bounds,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}