#include <windows.h>
#include <exdisp.h>
#include <shlobj.h>
#include "choice_explorer.h"

#define SAFE_RELEASE(p) do { if (p) { (p)->lpVtbl->Release(p); (p) = NULL; } } while(0)

// =========================================================================
// BACKEND: All the Win32/COM "mess" is isolated here and returns a clean list.
// =========================================================================

static BOOL IsValidExplorerWindow(HWND hwnd) {
    if (!hwnd || !IsWindowVisible(hwnd)) return FALSE;
    wchar_t className[64];
    GetClassNameW(hwnd, className, 64);
    return (wcscmp(className, L"Progman") != 0 && wcscmp(className, L"WorkerW") != 0);
}

static BOOL GetExplorerPath(IWebBrowserApp* pBrowser, HWND hwnd, wchar_t* path, DWORD maxLen) {
    IServiceProvider* sp = NULL;
    IShellBrowser* sb = NULL;
    IShellView* sv = NULL;
    IFolderView* fv = NULL;
    IPersistFolder2* pf = NULL;
    PIDLIST_ABSOLUTE pidl = NULL;
    BOOL success = FALSE;

    if (SUCCEEDED(pBrowser->lpVtbl->QueryInterface(pBrowser, &IID_IServiceProvider, (void**)&sp)) &&
        SUCCEEDED(sp->lpVtbl->QueryService(sp, &SID_STopLevelBrowser, &IID_IShellBrowser, (void**)&sb)) &&
        SUCCEEDED(sb->lpVtbl->QueryActiveShellView(sb, &sv)) &&
        SUCCEEDED(sv->lpVtbl->QueryInterface(sv, &IID_IFolderView, (void**)&fv)) &&
        SUCCEEDED(fv->lpVtbl->GetFolder(fv, &IID_IPersistFolder2, (void**)&pf)) &&
        SUCCEEDED(pf->lpVtbl->GetCurFolder(pf, &pidl)) &&
        SHGetPathFromIDListW(pidl, path))
    {
        success = TRUE;
        if (wcslen(path) == 2 && path[1] == L':') {
            wcscat_s(path, maxLen, L"\\");
        }
    }

    if (pidl) CoTaskMemFree(pidl);
    SAFE_RELEASE(pf); SAFE_RELEASE(fv); SAFE_RELEASE(sv);
    SAFE_RELEASE(sb); SAFE_RELEASE(sp);

    if (!success || path[0] == L'\0') {
        GetWindowTextW(hwnd, path, maxLen);
        wchar_t* suffix = wcsstr(path, L" - Explorateur");
        if (suffix) *suffix = L'\0';
    }

    OutputDebugStringW(path);
    return (path[0] != L'\0' && wcscmp(path, L" ") != 0);
}

int Backend_GetOpenExplorers(ExplorerInfo* outList, int maxCapacity) {
    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) return 0;

    IShellWindows* pSHWindows = NULL;
    if (FAILED(CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void**)&pSHWindows))) {
        CoUninitialize();
        return 0;
    }

    long count = 0;
    pSHWindows->lpVtbl->get_Count(pSHWindows, &count);
    int foundCount = 0;

    for (long i = 0; i < count && foundCount < maxCapacity; i++) {
        VARIANT v;
        VariantInit(&v);
        V_VT(&v) = VT_I4;
        V_I4(&v) = i;

        IDispatch* pDisp = NULL;
        if (FAILED(pSHWindows->lpVtbl->Item(pSHWindows, v, &pDisp)) || !pDisp) continue;

        IWebBrowserApp* pBrowser = NULL;
        if (SUCCEEDED(pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void**)&pBrowser))) {
            HWND hwnd = NULL;
            pBrowser->lpVtbl->get_HWND(pBrowser, (SHANDLE_PTR*)&hwnd);

            if (IsValidExplorerWindow(hwnd)) {
                wchar_t path[MAX_PATH] = { 0 };
                if (GetExplorerPath(pBrowser, hwnd, path, MAX_PATH)) {
                    wcsncpy_s(outList[foundCount].chemin, MAX_PATH, path, _TRUNCATE);
                    outList[foundCount].hwndExplorateur = hwnd;
                    outList[foundCount].hThumbnail = NULL;
                    outList[foundCount].hwndPlaceholder = NULL;
                    foundCount++;
                }
            }
            SAFE_RELEASE(pBrowser);
        }
        SAFE_RELEASE(pDisp);
    }

    SAFE_RELEASE(pSHWindows);
    CoUninitialize();
    return foundCount;
}

wchar_t* Backend_ExctractFilePath(IShellItem* pItem) {
    PWSTR pszFolderPath = NULL;
    wchar_t* value = NULL;

    HRESULT hr = pItem->lpVtbl->GetDisplayName(pItem, SIGDN_FILESYSPATH, &pszFolderPath);
    if (SUCCEEDED(hr))
    {
        value = _wcsdup(pszFolderPath);
        CoTaskMemFree(pszFolderPath);
    }
    pItem->lpVtbl->Release(pItem);

    return value;
}

IShellItem* Backend_DeployDialogSelector(IFileOpenDialog* pDlg) {
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