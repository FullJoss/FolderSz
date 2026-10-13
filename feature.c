#include "pch.h"
#include "feature.h"
#include "list_view.h"
#include "cache.h"
#include "dialog.h"

static ListFile listFile = { NULL, 0, 0 };
wchar_t *current_path = NULL;


static inline wchar_t* ExctractFilePath(IShellItem* pItem) {
    PWSTR pszFolderPath = NULL;
    wchar_t* value = NULL;
    // 7. Extraire le chemin d'accès sous forme de chaîne de caractères
    HRESULT hr = pItem->lpVtbl->GetDisplayName(pItem, SIGDN_FILESYSPATH, &pszFolderPath);
    if (SUCCEEDED(hr))
    {
        value = _wcsdup(pszFolderPath);
        CoTaskMemFree(pszFolderPath);
    }
    pItem->lpVtbl->Release(pItem);

    return value;
}

static inline IShellItem* DeployDialogSelector(IFileOpenDialog* pDlg) {
    DWORD dwOptions;
    pDlg->lpVtbl->GetOptions(pDlg, &dwOptions);
    pDlg->lpVtbl->SetOptions(pDlg, dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    HRESULT hr = pDlg->lpVtbl->Show(pDlg, mainHwnd);

    IShellItem* pItem = NULL;
    if (SUCCEEDED(hr))
    {
        hr = pDlg->lpVtbl->GetResult(pDlg, &pItem);
    }
    return pItem;
}

wchar_t *load_folder()
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
            IShellItem* pItem = DeployDialogSelector(pDlg);
            if (pItem)
            {
                value = ExctractFilePath(pItem);
            }
            pDlg->lpVtbl->Release(pDlg);
        }

        CoUninitialize();
    }

    return value;
}

static BOOL FilterSpecificPath(const wchar_t *path)
{
    if (_wcsicmp(path, L"C:\\Windows") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\Program Files") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\Program Files (x86)") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\ProgramData") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\$Recycle.Bin") == 0)
        return TRUE;

    return FALSE;
}

static BOOL Manage_max_capacity()
{
    int capacity = listFile.capacity;
    int count = listFile.count;

    if (capacity < count || capacity == 0)
    {
        capacity = 2 * capacity + 1;

        ElementFile *tmp = (ElementFile *)realloc(listFile.file, capacity * sizeof(ElementFile));

        if (tmp == NULL)
        {
            listFile.capacity = 0;

            if (listFile.file) 
                free(listFile.file);

            MessageBox_View(L"Erreur : Allocation mémoire échouée !");
            return FALSE;
        }

        listFile.file = tmp;
        listFile.capacity = capacity;
    }
    return TRUE;
}
ListFile* ScanFilePath(const wchar_t *dossierPath)
{
    listFile.count = 0;

    SetWindowTextW(mainHwnd, dossierPath);

    // GDI/Win32 requiert un masque de recherche (ex: "C:\MonDossier\*")
    wchar_t masque[MAX_PATH];
    swprintf_s(masque, MAX_PATH, L"%s\\*", dossierPath);

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileExW(masque, FindExInfoBasic, &findData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

    SHFILEINFOW sfiFile = {0};

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
                continue;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                continue;
            }

            listFile.count++;
            if (!Manage_max_capacity())
                break;

            // Stocker les données
            ElementFile *element = &listFile.file[listFile.count - 1];
            wcscpy_s(element->name, MAX_PATH, findData.cFileName);

            wchar_t cheminComplet[MAX_PATH];
            PathCombineW(cheminComplet, dossierPath, findData.cFileName);

            DWORD filterMask = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

            if (FilterSpecificPath(cheminComplet) || (findData.dwFileAttributes & filterMask) == filterMask)
            {
                AddFlagsElement(element, ElementSystem);

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    AddFlagsElement(element, ElementDirectory);
                else RemoveFlagsElement(element, ElementDirectory);

                element->size = 0;
            }
            else if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                RemoveFlagsElement(element, ElementSystem);
                AddFlagsElement(element, ElementDirectory);
                element->size = SearchFolderSize(cheminComplet, findData.ftLastWriteTime);
            }
            else
            {
                RemoveFlagsElement(element, ElementSystem);
                RemoveFlagsElement(element, ElementDirectory);
                element->size = ((ULONGLONG)findData.nFileSizeHigh << 32) | findData.nFileSizeLow;
            }

            SHGetFileInfoW(
                cheminComplet,
                findData.dwFileAttributes,
                &sfiFile,
                sizeof(sfiFile),
                SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

            element->indexIcon = sfiFile.iIcon;

        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    return &listFile;
}