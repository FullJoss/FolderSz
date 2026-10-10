#include "list_view.h"
#include "feature.h"
#include "theme.h"
#include <commctrl.h>

HWND list_view_hwnd = NULL;

// Variables globales pour mémoriser le sens du tri
BOOL g_TriCroissant = TRUE;
int g_ColonneTriee = 0; // -1 = aucun, 0 = Nom, 1 = Taille

// 1. Comparaison pour le tri par NOM (Alphabétique Unicode)
static int ComparerNom(const void *a, const void *b)
{
    ElementFile *elemA = (ElementFile *)a;
    ElementFile *elemB = (ElementFile *)b;

    BOOL isDirA = !!(elemA->flags & ElementDirectory);
    BOOL isDirB = !!(elemB->flags & ElementDirectory);

    if (isDirA ^ isDirB)
    {
        return isDirB - isDirA;
    }

    int resultat = _wcsicmp(elemA->name, elemB->name);
    return g_TriCroissant ? resultat : -resultat;
}

// Comparaison pour le tri par TAILLE (Numérique 64 bits)
static int ComparerTaille(const void *a, const void *b)
{
    ElementFile *elemA = (ElementFile *)a;
    ElementFile *elemB = (ElementFile *)b;

    BOOL isDirA = !!(elemA->flags & ElementDirectory);
    BOOL isDirB = !!(elemB->flags & ElementDirectory);

    if (isDirA ^ isDirB)
    {
        return isDirB - isDirA;
    }

    int diff = (elemA->size > elemB->size) - (elemA->size < elemB->size);
    int masksign = 0 - (!g_TriCroissant);
    return (diff ^ masksign) - masksign;
}

void init_list_view(int x, int y, int width, int height)
{
    HINSTANCE instance = GetModuleHandle(NULL);
    list_view_hwnd = CreateWindowExW(
        0, WC_LISTVIEWW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA, // <-- LVS_OWNERDATA active le mode virtuel
        x, y, width, height,
        mainHwnd, (HMENU)IDC_MY_LISTVIEW, instance, NULL);

    SetWindowTheme(list_view_hwnd, L"Explorer", NULL);

    int ntier = (width - 30) / 3;
    int fill = (width - 30) - ntier;

    LVCOLUMNW lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = L"Nom du fichier";
    lvc.cx = fill;
    SendMessageW(list_view_hwnd, LVM_INSERTCOLUMNW, 0, (LPARAM)&lvc);

    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = L"Taille";
    lvc.cx = ntier;
    SendMessageW(list_view_hwnd, LVM_INSERTCOLUMNW, 1, (LPARAM)&lvc);

    ListView_SetExtendedListViewStyle(list_view_hwnd, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    set_listview_theme(list_view_hwnd);

    SHFILEINFOW sfi = {0};

    // 2. Demander le handle de la liste d'icônes système (petites icônes 16x16)
    HIMAGELIST hSystemImageList = (HIMAGELIST)SHGetFileInfoW(
        L"C:\\",
        FILE_ATTRIBUTE_DIRECTORY,
        &sfi,
        sizeof(sfi),
        SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);

    // 3. Lier cette liste d'images globale à votre ListView
    if (hSystemImageList != NULL)
    {
        ListView_SetImageList(list_view_hwnd, hSystemImageList, LVSIL_SMALL);
    }
}

BOOL List_View_Msg_ClickHeader(LPARAM lParam)
{
    LPNMHDR lpnmh = (LPNMHDR)lParam;
    // Détecter le clic sur l'entête du ListView principal
    if (lpnmh->idFrom == IDC_MY_LISTVIEW && lpnmh->code == LVN_COLUMNCLICK)
    {
        LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
        int colonneCliquee = pnmv->iSubItem;

        // Si on clique sur la même colonne, on inverse le sens.
        // Si on change de colonne, on repasse par défaut en mode croissant.
        if (g_ColonneTriee == colonneCliquee)
        {
            g_TriCroissant = !g_TriCroissant;
        }
        else
        {
            g_ColonneTriee = colonneCliquee;
            g_TriCroissant = TRUE;
        }

        // Exécuter le bon tri selon la colonne cliquée
        if (colonneCliquee == 0)
        {
            // Colonne 0 = Nom du fichier
            qsort(list_view_data, list_view_data_count, sizeof(ElementFile), ComparerNom);
        }
        else if (colonneCliquee == 1)
        {
            // Colonne 1 = Taille du fichier / dossier
            qsort(list_view_data, list_view_data_count, sizeof(ElementFile), ComparerTaille);
        }

        // --- APPLIQUER LES CHANGEMENTS À L'ÉCRAN ---
        // On indique au ListView virtuel que sa mémoire cache a changé
        // LVM_REDRAWITEMS rafraîchit la plage de lignes de manière instantanée
        SendMessage(list_view_hwnd, LVM_REDRAWITEMS, 0, list_view_data_count - 1);

        // Forcer la mise à jour graphique globale (barres de défilement comprises)
        InvalidateRect(list_view_hwnd, NULL, TRUE);
        UpdateWindow(list_view_hwnd);

        return TRUE;
    }
    return FALSE;
}
BOOL List_View_Msg_GetInfo(LPARAM lParam)
{
    NMHDR *plvdi = (NMHDR *)lParam;

    // Le ListView réclame du texte pour une cellule !
    if (plvdi->idFrom == IDC_MY_LISTVIEW && plvdi->code == LVN_GETDISPINFOW)
    {
        NMLVDISPINFOW *plvdi = (NMLVDISPINFOW *)lParam;
        int index = plvdi->item.iItem; // Le numéro de la ligne demandée

        if (index < list_view_data_count)
        {
            // Si le contrôle veut le texte de la colonne 0 (Nom)
            if (plvdi->item.mask & LVIF_IMAGE)
            {
                plvdi->item.iImage = list_view_data[index].indexIcon;
            }
            if (plvdi->item.mask & LVIF_TEXT)
            {
                if (plvdi->item.iSubItem == 0)
                {
                    // On lui pointe directement l'adresse de notre cache en RAM
                    plvdi->item.pszText = list_view_data[index].name;
                    return TRUE;
                }
                else if (plvdi->item.iSubItem == 1)
                {
                    if (!(list_view_data[index].flags & ElementSystem))
                    {
                        ULONGLONG tailleEnOctets = list_view_data[index].size;
                        StrFormatByteSizeW((ULONGLONG)tailleEnOctets, plvdi->item.pszText, plvdi->item.cchTextMax);
                    }
                    else
                    {
                        plvdi->item.pszText = L"SystemFile";
                    }
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

void List_View_MAJ()
{
    SendMessageW(list_view_hwnd, LVM_SETITEMCOUNT, (WPARAM)list_view_data_count, 0);
    InvalidateRect(list_view_hwnd, NULL, TRUE);
    qsort(list_view_data, list_view_data_count, sizeof(ElementFile), ComparerNom);
}
BOOL List_View_Notification(LPARAM lParam)
{
    BOOL r = List_View_Msg_ClickHeader(lParam);

    r |= List_View_Msg_GetInfo(lParam);

    return r;
}