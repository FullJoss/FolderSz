#include "navigation.h"
#include "feature.h"
#include "list_view.h"
#include "dialog.h"
#include "backend.h"

static void BackParentDirectory(wchar_t* path);
static inline size_t LastAntiSlashSearch(wchar_t* path, size_t len);
static inline void CutPath(wchar_t* path, size_t index);

void OnDoubleClickRow(HWND hwnd, ListFileView* lfv) {

    ElementFile* elem = (char*)lfv->rows + lfv->indexRowSelect * sizeof(ElementFile);

    wchar_t buffer[MAX_PATH];
    wchar_t bufferCurrentPath[MAX_PATH];

    int lengthTitle = GetWindowTextLengthW(hwnd);
    GetWindowTextW(hwnd, bufferCurrentPath, MAX_PATH);

    if (elem->flags & ElementParentDirectory) {

        BackParentDirectory(bufferCurrentPath);

        ListFile* lf = ScanFilePath(bufferCurrentPath, lfv);
        UpdateListFileView(hwnd, lfv, lf, TRUE);
        return;
    }
    else 
        PathCombineW(buffer, bufferCurrentPath, elem->name);

    if (elem->flags & ElementCache || elem->flags & ElementPartition) {
        return;
    }
    else if (elem->flags & ElementDirectory) {

        if (elem->flags & ElementSystem) {
            BOOL user_has_accepted = MessageBox_Confirm(L"This is a system folder, do you want to continue?", FALSE);
            if (!user_has_accepted)
                return;
        }

        ListFile* lf = ScanFilePath(buffer, lfv);
        UpdateListFileView(hwnd, lfv, lf, TRUE);
    }
    else {
        SHELLEXECUTEINFOW sei = GetShellExecInfo(hwnd, buffer);
        BOOL result = ShellExecuteExW(&sei);

        if (!result) {
            MessageBox_Error(L"Error to open this file", L"Execution error");
        }
    }
}

static void BackParentDirectory(wchar_t* path) {
    size_t len = wcslen(path);
    if (len <= 3) return; // 3 is "C:\"

    while (len > 3 && path[len - 1] == L'\\') {
        len--;
    }

    size_t iLastSlash = LastAntiSlashSearch(path, len);

    if (iLastSlash > 0) {
        CutPath(path, iLastSlash);
    }
}

static inline size_t LastAntiSlashSearch(wchar_t *path, size_t len) {
    size_t iLastSlash = 0;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == L'\\') {
            iLastSlash = i;
        }
    }

    return iLastSlash;
}

static inline void CutPath(wchar_t* path, size_t index) {
    if (index == 2 && path[1] == L':') {
        path[3] = L'\0';
    }
    else {
        path[index] = L'\0';
    }
}
