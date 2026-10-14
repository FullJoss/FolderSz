#pragma once
#include <windows.h>
#include <exdisp.h>
#include <shlobj.h>
#include "choice_explorer.h"

int Backend_GetOpenExplorers(ExplorerInfo* outList, int maxCapacity);
inline wchar_t* Backend_ExctractFilePath(IShellItem* pItem);
inline IShellItem* Backend_DeployDialogSelector(IFileOpenDialog* pDlg);