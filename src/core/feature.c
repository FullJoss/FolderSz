/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#include "pch.h"
#include "feature.h"
#include "list_view.h"
#include "cache.h"
#include "dialog.h"
#include "backend.h"
#include "tick.h"

wchar_t *current_path = NULL;

static inline void PushData(wchar_t* pathElement, ElementFile* element, WIN32_FIND_DATAW* data);
static inline void AttachIcon(ElementFile* element, wchar_t* path, SHFILEINFOW* sfiFile, WIN32_FIND_DATAW* data);
static BOOL FilterSpecificPath(const wchar_t* path);
static inline void IdentifyFile(ElementFile* element, WIN32_FIND_DATAW* data);
static inline void IdentifyFolder(ElementFile* element, WIN32_FIND_DATAW* data);
static inline void IdentifyFileSystem(ElementFile* element, WIN32_FIND_DATAW* data);
static inline HCURSOR ChangeCursor();
static inline void RestorCursor(HCURSOR hOldCursor);
static void BackParentDirectory(wchar_t* path);

wchar_t *LoadFolderDialog()
{
    wchar_t *value = NULL;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pDlg = NULL;

        hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                              &IID_IFileOpenDialog, (void **)&pDlg);

        if (SUCCEEDED(hr))
        {
            IShellItem* pItem = Backend_DeployDialogSelector(pDlg);
            if (pItem)
            {
                value = Backend_ExctractFilePath(pItem);
            }
            pDlg->lpVtbl->Release(pDlg);
        }

        CoUninitialize();
    }

    return value;
}




void AsyncScanFolders(void* arg) {
    ScanArg* param = (ScanArg*)arg;
    FileVec* lf = param->lf;
    wchar_t* path = param->path;

    wchar_t completePath[MAX_PATH];
    for (size_t i = 0; i < lf->count; i++) {
        ElementFile* element = lf->data + i;
        if (element->flags & ElementDirWithoutSize) {
            PathCombineW(completePath, path, element->name);
            element->size = SearchFolderSize(completePath, element->lastUpdate);
            RemoveFlagsElement(element, ElementDirWithoutSize);
        }
    }
}

FileVec* ScanFilePath(const wchar_t *dossierPath)
{
    HCURSOR hOldCursor = ChangeCursor();

    static FileVec listFile = { 0 };
    listFile.count = 0;

    wchar_t masque[MAX_PATH];
    swprintf_s(masque, MAX_PATH, L"%s\\*", dossierPath);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileExW(masque, FindExInfoBasic, &findData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
    SHFILEINFOW sfiFile = {0};

    InitTick();

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (wcscmp(findData.cFileName, L".") == 0)
                continue;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                continue;
            }

            VecGrow(&listFile);
            ElementFile* element = VecLast(&listFile);
            
            element->flags = ElementInvalid;
            element->size = 0;

            wcscpy_s(element->name, MAX_PATH, findData.cFileName);

            wchar_t completePath[MAX_PATH];
            PathCombineW(completePath, dossierPath, findData.cFileName);

            if (wcscmp(findData.cFileName, L"..") == 0) {
                element->flags = ElementParentDirectory | ElementDirectory;
            }
            else
            {
                PushData(completePath, element, &findData);
            }

            AttachIcon(element, completePath, &sfiFile, &findData);

        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    RestorCursor(hOldCursor);

    return &listFile;
}








static inline void PushData(wchar_t *pathElement, ElementFile* element, WIN32_FIND_DATAW *data) {

    DWORD isHiddenSystem = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

    if (FilterSpecificPath(pathElement) || (data->dwFileAttributes & isHiddenSystem) == isHiddenSystem)
    {
        IdentifyFileSystem(element, data);
    }
    else if (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        IdentifyFolder(element, data);
    }
    else
    {
        IdentifyFile(element, data);
    }
}

static inline void AttachIcon(ElementFile* element, wchar_t *path, SHFILEINFOW* sfiFile, WIN32_FIND_DATAW *data) {
    SHGetFileInfoW(
        path,
        data->dwFileAttributes,
        sfiFile,
        sizeof(*sfiFile),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    element->indexIcon = sfiFile->iIcon;
}


static BOOL FilterSpecificPath(const wchar_t* path)
{
    if (_wcsicmp(path, L"C:\\Windows") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\ProgramData") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\$Recycle.Bin") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\WinREAgent") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\Config.Msi") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\System Volume Information") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\MSOCache") == 0)
        return TRUE;

    return FALSE;
}

static inline void IdentifyFileSystem(ElementFile* element, WIN32_FIND_DATAW* data) {
    AddFlagsElement(element, ElementSystem);

    if (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        AddFlagsElement(element, ElementDirectory);
}

static inline void IdentifyFolder(ElementFile* element, WIN32_FIND_DATAW* data) {
    AddFlagsElement(element, ElementDirectory);
    AddFlagsElement(element, ElementDirWithoutSize);
    element->lastUpdate = data->ftLastWriteTime;
}

static inline void IdentifyFile(ElementFile* element, WIN32_FIND_DATAW* data) {
    element->size = ((ULONGLONG)data->nFileSizeHigh << 32) | data->nFileSizeLow;
}

static inline HCURSOR ChangeCursor() {
    HCURSOR hWaitCursor = LoadCursor(NULL, IDC_WAIT);
    HCURSOR hOldCursor = SetCursor(hWaitCursor);

    SetCapture(GetDesktopWindow());

    return hOldCursor;
}

static inline void RestorCursor(HCURSOR hOldCursor) {
    ReleaseCapture();
    SetCursor(hOldCursor);
}
