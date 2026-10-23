#include "navigation.h"
#include "feature.h"
#include "list_view.h"
#include "dialog.h"
#include "backend.h"

static void BackParentDirectory(wchar_t* path);
static inline size_t LastAntiSlashSearch(wchar_t* path, size_t len);
static inline void CutPath(wchar_t* path, size_t index);
static inline void ExploreDir(HWND hwnd, wchar_t* path, ListFileView* lfv);
static inline void ExploreOrExecute(HWND hwnd, ElementFile* elem, wchar_t* buffer, ListFileView* lfv);

void OnDoubleClickRow(HWND hwnd, ListFileView* lfv) {

    ElementFile* elem = (ElementFile*)lfv->rows + lfv->indexRowSelect;

    if (elem->flags & (ElementCache | ElementPartition)) {
        return;
    }

    wchar_t buffer[MAX_PATH];

    wchar_t bufferCurrentPath[MAX_PATH];
    GetWindowTextW(hwnd, bufferCurrentPath, MAX_PATH);

    if (elem->flags & ElementParentDirectory) {
        BackParentDirectory(bufferCurrentPath);
        ExploreDir(hwnd, bufferCurrentPath, lfv);
        return;
    }
    
    PathCombineW(buffer, bufferCurrentPath, elem->name);
    
    ExploreOrExecute(hwnd, elem, buffer, lfv);
}

static void BackParentDirectory(wchar_t* path) {
    size_t len = wcslen(path);

    const size_t minLen = wcslen(L"C:\\");

    if (len <= minLen) return;

    while (len > minLen && path[len - 1] == L'\\') {
        len--;
    }

    size_t iLastSlash = LastAntiSlashSearch(path, len);

    if (iLastSlash > 0) {
        CutPath(path, iLastSlash);
    }
}

static inline size_t LastAntiSlashSearch(wchar_t *path, size_t len) {
    for (size_t i = len; i > 0; i--) {
        if (path[i] == L'\\') {
            return i;
        }
    }

    return 0;
}

static inline void ExploreOrExecute(HWND hwnd, ElementFile* elem, wchar_t* buffer, ListFileView* lfv) {
    if (elem->flags & ElementDirectory) {

        if (elem->flags & ElementSystem) {
            BOOL user_has_accepted = MessageBox_Confirm(L"This is a system folder, do you want to continue?", FALSE);
            if (!user_has_accepted)
                return;
        }

        ExploreDir(hwnd, buffer, lfv);
    }
    else {
        SHELLEXECUTEINFOW sei = GetShellExecInfo(hwnd, buffer);
        BOOL result = ShellExecuteExW(&sei);

        if (!result) {
            MessageBox_Error(L"Error to open this file", L"Execution error");
        }
    }
}

static inline void ExploreDir(HWND hwnd, wchar_t* path, ListFileView* lfv) {
    ListFile* lf = ScanFilePath(path, lfv);
    UpdateListFileView(hwnd, lfv, lf, TRUE);
}

static inline void CutPath(wchar_t* path, size_t index) {
    if (index == 2 && path[1] == L':') {
        path[3] = L'\0';
    }
    else {
        path[index] = L'\0';
    }
}
