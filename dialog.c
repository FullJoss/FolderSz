#include "pch.h"
#include "dialog.h"

void MessageBox_View(const wchar_t *text)
{
    MessageBoxW(mainHwnd, text, L"Attention !", MB_OK | MB_ICONINFORMATION);
}

int MessageBox_Confirm(const wchar_t *text)
{
    return MessageBoxW(
        mainHwnd,                     // 1. Handle de la fenêtre parente (ou NULL)
        text,                         // 2. Le texte du message
        L"Confirmation",              // 3. Le titre de la boîte de dialogue
        MB_OKCANCEL | MB_ICONQUESTION // 4. Les boutons et l'icône
    );
}