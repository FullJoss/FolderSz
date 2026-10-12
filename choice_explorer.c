#include "choice_explorer.h"
#include "theme.h"

#pragma comment(lib, "dwmapi.lib")

static Label g_labels[10];
static ExplorateurOuvert g_Explorers[10];
static int g_totalTrouve = 0;
static int g_indexSelection = -1;
static HWND mhwnd;
static WNDPROC g_OldStaticProc = NULL;

#define ID_LISTCHOIX 205
#define MAX_EXPLORERS 10
#define ID_MIGNATURE_BASE 6000

BOOL create_choice_explorer(HWND hwndParent, wchar_t *outPath, DWORD maxLen)
{
    fill_g_Explorers();

    if (g_totalTrouve == 0)
        return FALSE;
    if (g_totalTrouve == 1)
    { // Pas besoin de choix s'il n'y en a qu'un
        wcsncpy_s(outPath, maxLen, g_Explorers[0].chemin, _TRUNCATE);
        return TRUE;
    }

    g_indexSelection = -1;
    HINSTANCE hInst = GetModuleHandle(NULL);

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = AltTabChoixProc;
    wc.hInstance = hInst;
    wc.hbrBackground = CreateSolidBrush(RGB(28, 28, 28)); // Fond gris anthracite très sombre
    wc.lpszClassName = L"ModernAltTabClass";
    wc.style = CS_DBLCLKS; // Gérer le double-clic
    RegisterClassW(&wc);

    int colonnes = (g_totalTrouve > 3) ? 3 : g_totalTrouve;
    int lignes = (g_totalTrouve + 2) / 3;

    // Ajustement de la taille globale par rapport aux nouvelles cases de 260x160
    int winW = 30 + colonnes * 290 + 30;
    int winH = 50 + lignes * 210 + 30;

    // Centrage à l'écran
    int posX = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

    // --- CORRECTION VITALE : Ajout de WS_VISIBLE et vérification ---
    HWND hwndPopup = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"ModernAltTabClass", L"Choix Dossier",
        WS_POPUP | WS_BORDER | WS_VISIBLE, // WS_VISIBLE force Windows à l'afficher immédiatement
        posX, posY, winW, winH,
        hwndParent, NULL, hInst, NULL);

    if (hwndPopup == NULL)
    {
        // Si la création échoue, on nettoie et on sort pour éviter le blocage
        UnregisterClassW(L"ModernAltTabClass", hInst);
        return FALSE;
    }

    ShowWindow(hwndPopup, SW_SHOW);
    UpdateWindow(hwndPopup);

    // Boucle de message modale locale

    printf("Debug du message explorerrs");
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterClassW(L"ModernAltTabClass", hInst);

    if (g_indexSelection != -1)
    {
        wcsncpy_s(outPath, maxLen, g_Explorers[g_indexSelection].chemin, _TRUNCATE);
        return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK VignetteSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int index = (int)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    HWND hwndParent = GetParent(hwnd);

    switch (msg)
    {
    case WM_MOUSEMOVE:
    {
        // Demander à Windows de nous avertir quand la souris partira (WM_MOUSELEAVE)
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        TrackMouseEvent(&tme);

        // Forcer la fenêtre parente à redessiner les bordures de survol
        InvalidateRect(hwndParent, NULL, FALSE);
        break;
    }
    case WM_MOUSELEAVE:
    {
        // La souris a quitté la vignette : on force TOUT DE SUITE un rafraîchissement complet
        // Le TRUE permet d'effacer les anciens dessins (y compris la bordure bleue parasite)
        InvalidateRect(hwndParent, NULL, TRUE);
        break;
    }
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    {
        g_indexSelection = index;
        DestroyWindow(hwndParent);
        return 0;
    }
    }
    return CallWindowProcW(g_OldStaticProc, hwnd, msg, wParam, lParam);
}

// WndProc principale de l'interface "Alt+Tab"
LRESULT CALLBACK AltTabChoixProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));

        HINSTANCE hInst = GetModuleHandle(NULL);

        // --- NOUVELLES DIMENSIONS POUR UNE GRILLE PLUS GRANDE ET PLUS NETTE ---
        for (int i = 0; i < g_totalTrouve; i++)
        {
            int col = i % 3;
            int row = i / 3;

            // On élargit l'espace : cases de 260x160 pixels avec des marges de 30px
            int x = 30 + col * (260 + 30);
            int y = 50 + row * (160 + 50);

            wchar_t nomDossier[MAX_PATH] = {0};

            // Si la chaîne contient un chemin complet (avec un antislash au milieu)
            wchar_t *dernierSlash = wcsrchr(g_Explorers[i].chemin, L'\\');
            if (dernierSlash != NULL && *(dernierSlash + 1) != L'\0')
            {
                wcsncpy_s(nomDossier, MAX_PATH, dernierSlash + 1, _TRUNCATE);
            }
            else
            {
                // Si c'est une racine (C:\) ou un titre direct (Ce PC, Accueil), on garde la chaîne brute
                wcsncpy_s(nomDossier, MAX_PATH, g_Explorers[i].chemin, _TRUNCATE);
            }

            // Création du cadre d'affichage agrandi
            HWND hwndPlace = CreateWindowExW(
                0, L"STATIC", L"",
                WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                x, y, 260, 160, hwnd, (HMENU)(INT_PTR)(ID_MIGNATURE_BASE + i), hInst, NULL);

            // Label textuel repositionné en dessous
           /* CreateWindowExW(
                0, L"STATIC", nomDossier,
                WS_CHILD | WS_VISIBLE | SS_CENTER | SS_ENDELLIPSIS,
                x, y + 165, 260, 20, hwnd, NULL, hInst, NULL);*/
            g_labels[i].x = x;
            g_labels[i].y = y + 165;
            wcscpy_s(g_labels[i].text, MAX_PATH, nomDossier);

            g_Explorers[i].hwndPlaceholder = hwndPlace;
            SetWindowLongPtrW(hwndPlace, GWLP_USERDATA, (LONG_PTR)i);

            if (!g_OldStaticProc)
            {
                g_OldStaticProc = (WNDPROC)GetWindowLongPtrW(hwndPlace, GWLP_WNDPROC);
            }
            SetWindowLongPtrW(hwndPlace, GWLP_WNDPROC, (LONG_PTR)VignetteSubclassProc);

            // --- CONFIGURATION DWM HAUTE DÉFINITION ---
            if (SUCCEEDED(DwmRegisterThumbnail(hwnd, g_Explorers[i].hwndExplorateur, &g_Explorers[i].hThumbnail)))
            {
                RECT rcDest = {x, y, x + 260, y + 160};

                DWM_THUMBNAIL_PROPERTIES props = {0};
                // L'ajout de DWM_TNP_SOURCECLIENTAREAONLY force Windows à chercher la texture HD d'origine
                props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_OPACITY;
                props.rcDestination = rcDest;
                props.fVisible = TRUE;
                props.opacity = 255;

                DwmUpdateThumbnailProperties(g_Explorers[i].hThumbnail, &props);
            }
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Dessiner le titre de la fenêtre en blanc
        SetTextColor(hdc, g_colors[IC_TEXT]);
        SetBkMode(hdc, TRANSPARENT);
        SelectObject(hdc, g_font);

        RECT rcTitle = {20, 10, 500, 35};
        DrawTextW(hdc, L"Sélectionnez la fenêtre de l'Explorateur à synchroniser :", -1, &rcTitle, DT_LEFT | DT_VCENTER);

        // Dessiner un cadre de surbrillance bleu si la souris survole une case
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);

        for (int i = 0; i < g_totalTrouve; i++)
        {
            RECT rc;
            GetWindowRect(g_Explorers[i].hwndPlaceholder, &rc);
            MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rc, 2);

            if (PtInRect(&rc, pt))
            {
                // Agrandir légèrement le rectangle pour faire une bordure extérieure
                InflateRect(&rc, 3, 3);
                HPEN hNewPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 225));
                // 1. Rendre le fond du rectangle transparent (pour ne pas remplir l'intérieur)
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                // 3. Sélectionner le nouveau pinceau dans le HDC (et sauvegarder l'ancien)
                HPEN hOldPen = (HPEN)SelectObject(hdc, hNewPen);
                Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                SelectObject(hdc, hOldPen);   // Remet l'ancien pinceau
                SelectObject(hdc, hOldBrush); // Remet l'ancienne brosse
                DeleteObject(hNewPen);        // Détruit le pinceau créé
            }

            TextOutW(hdc, g_labels[i].x, g_labels[i].y, g_labels[i].text, lstrlenW(g_labels[i].text));
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        { // Quitter avec Échap
            DestroyWindow(hwnd);
        }
        break;

    case WM_DESTROY:
        // 4. Nettoyage obligatoire des flux DWM pour éviter les fuites de VRAM
        for (int i = 0; i < g_totalTrouve; i++)
        {
            if (g_Explorers[i].hThumbnail)
            {
                DwmUnregisterThumbnail(g_Explorers[i].hThumbnail);
                g_Explorers[i].hThumbnail = NULL;
            }
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Déclaration de la fonction
void fill_g_Explorers(void)
{
    // 1. Initialiser le système COM (si ce n'est pas déjà fait dans WinMain)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return;
    // Réinitialiser le compteur global
    g_totalTrouve = 0;

    IShellWindows *pSHWindows = NULL;
    // Créer l'instance du gestionnaire de fenêtres du Shell Windows
    hr = CoCreateInstance(&CLSID_ShellWindows, NULL, CLSCTX_ALL, &IID_IShellWindows, (void **)&pSHWindows);
    if (FAILED(hr)) return;

    long count = 0;
    pSHWindows->lpVtbl->get_Count(pSHWindows, &count);

    // 2. Parcourir toutes les fenêtres ouvertes détectées par le système
    for (long i = 0; i < count && g_totalTrouve < MAX_EXPLORERS; i++)
    {
        VARIANT v;
        VariantInit(&v);
        V_VT(&v) = VT_I4;
        V_I4(&v) = i;

        IDispatch *pDisp = NULL;

        // Récupérer l'élément individuel
        if (SUCCEEDED(pSHWindows->lpVtbl->Item(pSHWindows, v, &pDisp)) && pDisp != NULL)
        {
            IWebBrowserApp *pBrowser = NULL;

            // Vérifier s'il s'agit bien d'une instance de l'Explorateur/Internet Explorer
            if (SUCCEEDED(pDisp->lpVtbl->QueryInterface(pDisp, &IID_IWebBrowserApp, (void **)&pBrowser)))
            {

                HWND hwndBrowser = NULL;
                // Extraction du handle de fenêtre (HWND) réel
                pBrowser->lpVtbl->get_HWND(pBrowser, (SHANDLE_PTR *)&hwndBrowser);

                // ... À l'intérieur de fill_g_Explorers(), après avoir récupéré hwndBrowser :
                if (hwndBrowser != NULL && IsWindowVisible(hwndBrowser))
                {
                    wchar_t className[64];
                    GetClassNameW(hwndBrowser, className, 64);

                    // Ignorer le bureau et les composants systèmes internes
                    if (wcscmp(className, L"Progman") == 0 || wcscmp(className, L"WorkerW") == 0)
                    {
                        pBrowser->lpVtbl->Release(pBrowser);
                        pDisp->lpVtbl->Release(pDisp);
                        continue;
                    }

                    wchar_t tempPath[MAX_PATH] = {0};
                    BOOL aUnChemin = FALSE;

                    // 1. Tenter d'obtenir le chemin par le Shell (Méthode classique)
                    IServiceProvider *pServiceProvider = NULL;
                    if (SUCCEEDED(pBrowser->lpVtbl->QueryInterface(pBrowser, &IID_IServiceProvider, (void **)&pServiceProvider)))
                    {
                        IShellBrowser *pShellBrowser = NULL;
                        if (SUCCEEDED(pServiceProvider->lpVtbl->QueryService(pServiceProvider, &SID_STopLevelBrowser, &IID_IShellBrowser, (void **)&pShellBrowser)))
                        {
                            IShellView *pShellView = NULL;
                            if (SUCCEEDED(pShellBrowser->lpVtbl->QueryActiveShellView(pShellBrowser, &pShellView)))
                            {
                                IFolderView *pFolderView = NULL;
                                if (SUCCEEDED(pShellView->lpVtbl->QueryInterface(pShellView, &IID_IFolderView, (void **)&pFolderView)))
                                {
                                    IPersistFolder2 *pPersistFolder = NULL;
                                    if (SUCCEEDED(pFolderView->lpVtbl->GetFolder(pFolderView, &IID_IPersistFolder2, (void **)&pPersistFolder)))
                                    {
                                        PIDLIST_ABSOLUTE pidl = NULL;
                                        if (SUCCEEDED(pPersistFolder->lpVtbl->GetCurFolder(pPersistFolder, &pidl)))
                                        {

                                            if (SHGetPathFromIDListW(pidl, tempPath))
                                            {
                                                aUnChemin = TRUE;
                                                // Sécurité pour les racines de disques (ex: C:)
                                                if (wcslen(tempPath) == 2 && tempPath[1] == L':')
                                                {
                                                    wcscat_s(tempPath, MAX_PATH, L"\\");
                                                }
                                            }
                                            CoTaskMemFree(pidl);
                                        }
                                        pPersistFolder->lpVtbl->Release(pPersistFolder);
                                    }
                                    pFolderView->lpVtbl->Release(pFolderView);
                                }
                                pShellView->lpVtbl->Release(pShellView);
                            }
                            pShellBrowser->lpVtbl->Release(pShellBrowser);
                        }
                        pServiceProvider->lpVtbl->Release(pServiceProvider);
                    }

                    // 2. LA SÉCURITÉ ABSOLUE : Si le chemin est resté vide, on prend le titre de la fenêtre !
                    if (!aUnChemin || tempPath[0] == L'\0')
                    {
                        GetWindowTextW(hwndBrowser, tempPath, MAX_PATH);

                        // Nettoyage cosmétique : l'Explorateur ajoute souvent " - Explorateur de fichiers" au titre
                        wchar_t *posSuffixe = wcsstr(tempPath, L" - Explorateur");
                        if (posSuffixe != NULL)
                        {
                            *posSuffixe = L'\0'; // On coupe la chaîne pour ne garder que le nom (ex: "Ce PC" ou "Accueil")
                        }
                    }

                    // 3. FILTRAGE FINAL : Si après tout ça la chaîne est ENCORE vide, on REJETTE la fenêtre
                    if (tempPath[0] != L'\0' && wcscmp(tempPath, L" ") != 0)
                    {
                        wcsncpy_s(g_Explorers[g_totalTrouve].chemin, MAX_PATH, tempPath, _TRUNCATE);
                        g_Explorers[g_totalTrouve].hwndExplorateur = hwndBrowser;
                        g_Explorers[g_totalTrouve].hThumbnail = NULL;
                        g_Explorers[g_totalTrouve].hwndPlaceholder = NULL;

                        g_totalTrouve++; // On n'incrémente QUE SI on a un vrai texte valide
                    }
                }

                pBrowser->lpVtbl->Release(pBrowser);
            }
            pDisp->lpVtbl->Release(pDisp);
        }
    }

    // Libération du gestionnaire Shell Windows
    pSHWindows->lpVtbl->Release(pSHWindows);
}
void AfficherMiniatureFenetre(HWND hwndVotreInterface, HWND hwndExplorateurSource, RECT zoneAffichage, HTHUMBNAIL *outThumbnail)
{
    // 1. Créer le lien visuel entre l'Explorateur et votre fenêtre
    if (SUCCEEDED(DwmRegisterThumbnail(hwndVotreInterface, hwndExplorateurSource, outThumbnail)))
    { // 1.3.1

        DWM_THUMBNAIL_PROPERTIES dtnp;
        dtnp.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_VISIBLE | DWM_TNP_OPACITY;
        dtnp.fVisible = TRUE;
        dtnp.opacity = 255;                 // 255 = Opaque, transparent = 0
        dtnp.rcDestination = zoneAffichage; // Les coordonnées en pixels de la case dans votre logiciel

        // 2. Mettre à jour les propriétés pour forcer l'affichage immédiat
        DwmUpdateThumbnailProperties(*outThumbnail, &dtnp); // 1.3.2
    }
}

void CloseChoiceExplorer()
{
    // À mettre absolument dans le message WM_DESTROY de votre fenêtre de choix :
    for (int i = 0; i < g_totalTrouve; i++)
    {
        if (g_Explorers[i].hThumbnail != NULL)
        {
            DwmUnregisterThumbnail(g_Explorers[i].hThumbnail);
            g_Explorers[i].hThumbnail = NULL;
        }
    }
}