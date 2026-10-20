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
#include <pathcch.h>

#pragma comment(lib, "Pathcch.lib")

static ListFile listFile = { NULL, 0, 0 };
wchar_t *current_path = NULL;

#define REFRESH_TIME_SECOND 0.5
static double m_scan_time = 0;

static inline void PushData(wchar_t* pathElement, ElementFile* element, WIN32_FIND_DATAW* data, ListFileView* lfv);
static inline void AttachIcon(ElementFile* element, wchar_t* path, SHFILEINFOW* sfiFile, WIN32_FIND_DATAW* data);
static BOOL FilterSpecificPath(const wchar_t* path);
static inline void IdentifyFile(ElementFile* element, WIN32_FIND_DATAW* data);
static inline void IdentifyFolder(wchar_t* pathElement, ElementFile* element, WIN32_FIND_DATAW* data, ListFileView* lfv);
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

static BOOL ManageMaxCapacity()
{
    int capacity = listFile.capacity;
    int count = listFile.count;

    if (capacity < count || capacity == 0)
    {
        capacity = 2 * capacity + 1;

        ElementFile *tmp = (ElementFile *)realloc(listFile.files, capacity * sizeof(ElementFile));

        if (tmp == NULL)
        {
            listFile.capacity = 0;

            if (listFile.files) 
                free(listFile.files);

            perror("listFile allocation failed!");
            return FALSE;
        }

        listFile.files = tmp;
        listFile.capacity = capacity;
    }
    return TRUE;
}

ListFile* ScanFilePath(const wchar_t *dossierPath, ListFileView* lfv)
{
    HCURSOR hOldCursor = ChangeCursor();

    listFile.count = 0;

    SetWindowTextW(mainHwnd, dossierPath);

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

            listFile.count++;
            if (!ManageMaxCapacity())
                break;

            ElementFile* element = &listFile.files[listFile.count - 1];
            
            element->flags = ElementInvalid;

            wcscpy_s(element->name, MAX_PATH, findData.cFileName);

            wchar_t cheminComplet[MAX_PATH];
            PathCchCombine(cheminComplet, MAX_PATH, dossierPath, findData.cFileName);

            if (wcscmp(findData.cFileName, L"..") == 0) {
                element->flags = ElementParentDirectory | ElementDirectory;
                element->size = 0;
            }
            else 
                PushData(cheminComplet, element, &findData, lfv);

            AttachIcon(element, cheminComplet, &sfiFile, &findData);

        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    RestorCursor(hOldCursor);

    return &listFile;
}

void RefreshView(ListFileView* lfv) {
    double deltaTime = GetElapsedTime();
    m_scan_time += deltaTime;

    if (m_scan_time >= REFRESH_TIME_SECOND) {
        UpdateListFileView(mainHwnd, lfv, &listFile, FALSE);
        m_scan_time -= REFRESH_TIME_SECOND;
    }
}

void OnDoubleClickRow(HWND hwnd, ListFileView* lfv) {

    ElementFile* elem = (char*)lfv->rows + lfv->indexRowSelect * sizeof(ElementFile);
    
    wchar_t buffer[MAX_PATH];
    wchar_t bufferCurrentPath[MAX_PATH];

    int lengthTitle = GetWindowTextLengthW(hwnd);
    GetWindowText(hwnd, &bufferCurrentPath, MAX_PATH);

    PathCchCombine(buffer, MAX_PATH, bufferCurrentPath, elem->name);

    if (elem->flags & ElementCache || elem->flags & ElementPartition) {
        return;
    }
    if (elem->flags & ElementParentDirectory) {
        HRESULT hr = PathCchRemoveFileSpec(bufferCurrentPath, MAX_PATH);

        if (SUCCEEDED(hr)) {
            ListFile* lf = ScanFilePath(bufferCurrentPath, lfv);
            UpdateListFileView(hwnd, lfv, lf, TRUE);
        }
    }
    else if (elem->flags & ElementDirectory) {
        ListFile* lf = ScanFilePath(buffer, lfv);
        UpdateListFileView(hwnd, lfv, lf, TRUE);
    }
    else {
        ShellExecute(hwnd, L"open", buffer, NULL, NULL, SW_SHOWNORMAL);
    }

    DEBUG_PRINT("CLICK DOUBLE !!!!");
    DEBUG_PRINT("bufer");
}

static inline void PushData(wchar_t *pathElement, ElementFile* element, WIN32_FIND_DATAW *data, ListFileView* lfv) {

    DWORD isHiddenSystem = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

    if (FilterSpecificPath(pathElement) || (data->dwFileAttributes & isHiddenSystem) == isHiddenSystem)
    {
        IdentifyFileSystem(element, data);
    }
    else if (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        IdentifyFolder(pathElement, element, data, lfv);
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

    return FALSE;
}

static inline void IdentifyFileSystem(ElementFile* element, WIN32_FIND_DATAW* data) {
    AddFlagsElement(element, ElementSystem);

    if (data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        AddFlagsElement(element, ElementDirectory);
    else RemoveFlagsElement(element, ElementDirectory);

    element->size = 0;
}

static inline void IdentifyFolder(wchar_t* pathElement, ElementFile* element, WIN32_FIND_DATAW* data, ListFileView* lfv) {
    RemoveFlagsElement(element, ElementSystem);
    AddFlagsElement(element, ElementDirectory);
    element->size = SearchFolderSize(pathElement, data->ftLastWriteTime, lfv);
}

static inline void IdentifyFile(ElementFile* element, WIN32_FIND_DATAW* data) {
    RemoveFlagsElement(element, ElementSystem);
    RemoveFlagsElement(element, ElementDirectory);
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

static void BackParentDirectory(wchar_t* path) {
    size_t len = wcslen(path);
    if (len <= 3) return; // Déjà à la racine (ex: "C:\") ou vide, on ne fait rien

    // 1. Si le chemin se termine par un ou plusieurs antislashs (ex: "C:\Dossier\")
    // on les ignore en réduisant virtuellement la taille de recherche
    while (len > 3 && path[len - 1] == L'\\') {
        len--;
    }

    // 2. Trouver le VRAI dernier antislash de séparation
    size_t indexDernierSlash = 0;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == L'\\') {
            indexDernierSlash = i;
        }
    }

    // 3. Appliquer la coupure au bon endroit
    if (indexDernierSlash > 0) {
        // Cas particulier : si le dernier slash est celui de la racine (ex: "C:\Dossier")
        if (indexDernierSlash == 2 && path[1] == L':') {
            path[3] = L'\0'; // On garde "C:\"
        }
        // Cas normal (ex: "C:\Dossier1\SousDossier2")
        else {
            path[indexDernierSlash] = L'\0'; // Coupe pile sur le '\'
        }
    }
}