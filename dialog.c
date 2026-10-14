#include "pch.h"
#include "dialog.h"

void MessageBox_View(const wchar_t *text)
{
    MessageBoxW(mainHwnd, text, L"Attention !", MB_OK | MB_ICONINFORMATION);
}

int MessageBox_Confirm(const wchar_t *text)
{
    return MessageBoxW(
        mainHwnd,                   
        text,                    
        L"Confirmation",      
        MB_OKCANCEL | MB_ICONQUESTION
    );
}