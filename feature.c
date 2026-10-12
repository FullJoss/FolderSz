#include "pch.h"
#include "feature.h"
#include "list_view.h"
#include "cache.h"
#include "dialog.h"

static ListFile listFile = { NULL, 0, 0 };
wchar_t *current_path = NULL;

LONGLONG start;

wchar_t *load_folder()
{
    wchar_t *value = NULL;
    // 1. Initialiser la bibliothèque COM (obligatoire)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pDlg = NULL;

        // 2. Créer l'instance du sélecteur de fichier
        hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
                              &IID_IFileOpenDialog, (void **)&pDlg);

        if (SUCCEEDED(hr))
        {
            DWORD dwOptions;
            // 3. Récupérer les options actuelles du dialogue
            pDlg->lpVtbl->GetOptions(pDlg, &dwOptions);

            // 4. ACTIVER LE MODE SÉLECTEUR DE DOSSIER (FOS_PICKFOLDERS)
            pDlg->lpVtbl->SetOptions(pDlg, dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

            // 5. Afficher la boîte de dialogue
            hr = pDlg->lpVtbl->Show(pDlg, mainHwnd);

            if (SUCCEEDED(hr))
            {
                IShellItem *pItem = NULL;
                // 6. Récupérer le dossier sélectionné par l'utilisateur
                hr = pDlg->lpVtbl->GetResult(pDlg, &pItem);

                if (SUCCEEDED(hr))
                {
                    PWSTR pszFolderPath = NULL;
                    // 7. Extraire le chemin d'accès sous forme de chaîne de caractères
                    hr = pItem->lpVtbl->GetDisplayName(pItem, SIGDN_FILESYSPATH, &pszFolderPath);
                    if (SUCCEEDED(hr))
                    {
                        value = _wcsdup(pszFolderPath);
                        CoTaskMemFree(pszFolderPath);
                    }
                    pItem->lpVtbl->Release(pItem);
                }
            }
            pDlg->lpVtbl->Release(pDlg);
        }
        // 8. Libérer la bibliothèque COM à la fin
        CoUninitialize();
    }

    return value;
}

int rename_file(const wchar_t *ancienChemin, const wchar_t *nouveauNom)
{
    if (!ancienChemin || ancienChemin[0] == L'\0' || !nouveauNom)
    {
        return 0; // Échec contrôlé (retourne faux)
    }

    // 1. Trouver le dernier antislash (\) pour isoler le dossier parent
    const wchar_t *dernierSlash = wcsrchr(ancienChemin, L'\\');
    if (!dernierSlash)
    {
        return -1; // Pas d'antislash trouvé = chemin invalide
    }

    // 2. Calculer le nombre exact de CARACTÈRES du chemin parent (incluant le '\')
    size_t nbCaracteresParent = dernierSlash - ancienChemin + 1;

    // 3. Calculer la taille du nouveau nom
    size_t nbCaracteresNouveauNom = wcslen(nouveauNom);

    // 4. Allouer la mémoire (Nombre de caractères total + 1 pour le '\0' final)
    size_t nbTotalCaracteres = nbCaracteresParent + nbCaracteresNouveauNom + 1;

    wchar_t *nouveauChemin = (wchar_t *)malloc(nbTotalCaracteres * sizeof(wchar_t));
    if (!nouveauChemin)
        return -1; // Échec du malloc

    // 5. Copier la partie parente (ex: "C:\Dossier\")
    wcsncpy_s(nouveauChemin, wcslen(ancienChemin), ancienChemin, nbCaracteresParent);

    // 6. Ajouter le '\0' manuellement pour que wcscat sache où se coller
    nouveauChemin[nbCaracteresParent] = L'\0';

    // 7. Coller le nouveau nom à la suite
    wcscat_s(nouveauChemin, nbCaracteresNouveauNom, nouveauNom);

    // 8. Renommer le dossier via l'API standard
    int resultat = MoveFileW(ancienChemin, nouveauChemin);

    // 9. Toujours libérer la mémoire allouée
    free(nouveauChemin);

    return (resultat == 1); // Retourne 0 en cas de succès
}

// Fonction pour déterminer si un dossier doit être ignoré
static BOOL filter_specific_path(const wchar_t *path)
{
    // 2. Filtres "en dur" des dossiers extrêmement lourds ou inaccessibles
    // _wcsicmp compare sans gérer la casse (insensible aux majuscules/minuscules)
    if (_wcsicmp(path, L"C:\\Windows") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\Program Files") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\Program Files (x86)") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\ProgramData") == 0)
        return TRUE;
    if (_wcsicmp(path, L"C:\\$Recycle.Bin") == 0)
        return TRUE; // Corbeille

    return FALSE; // Le dossier est valide, on peut le scanner
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

            if (filter_specific_path(cheminComplet) || (findData.dwFileAttributes & filterMask) == filterMask)
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