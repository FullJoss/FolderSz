/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#include "pch.h"
#include "dialog.h"

void MessageBox_Error(const wchar_t* text, const wchar_t* caption) {
    MessageBoxW(
        mainHwnd,
        text,
        caption,
        MB_OK | MB_ICONERROR | MB_TASKMODAL
    );
}

void MessageBox_View(const wchar_t *text)
{
    MessageBoxW(mainHwnd, text, L"Attention !", MB_OK | MB_ICONINFORMATION);
}

BOOL MessageBox_Confirm(const wchar_t* text, BOOL preferYes)
{
    UINT type = MB_YESNO | MB_ICONQUESTION;

    if (!preferYes)
        type |= MB_DEFBUTTON2;

    int r = MessageBoxW(
        mainHwnd,                   
        text,                    
        L"Confirmation",      
        type
    );

    return (r == IDYES);
}