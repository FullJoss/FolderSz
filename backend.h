#pragma once
#include <windows.h>
#include <exdisp.h>
#include <shlobj.h>
#include "type.h"
#include "choice_explorer.h"

int Backend_GetOpenExplorers(ExplorerInfo* outList, int maxCapacity);
wchar_t* Backend_ExctractFilePath(IShellItem* pItem);
IShellItem* Backend_DeployDialogSelector(IFileOpenDialog* pDlg);