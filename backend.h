#pragma once
#include <windows.h>
#include <exdisp.h>
#include <shlobj.h>
#include "choice_explorer.h"

int Backend_GetOpenExplorers(ExplorerInfo* outList, int maxCapacity);
