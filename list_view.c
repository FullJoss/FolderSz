#include "list_view.h"
#include "feature.h"
#include "theme.h"

HWND list_view_hwnd = NULL;
BOOL g_TriCroissant = TRUE;
ColumnListView g_ColonneTriee = COLUMN_LISTVIEW_NOM;

// Comparaison pour le tri par NOM (Alphabétique Unicode)
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
    return g_TriCroissant ? diff : -diff;
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

static BOOL List_View_Msg_ClickHeader(LPARAM lParam)
{
    LPNMHDR lpnmh = (LPNMHDR)lParam;
    // Détecter le clic sur l'entête du ListView principal
    if (lpnmh->idFrom == IDC_MY_LISTVIEW && lpnmh->code == LVN_COLUMNCLICK)
    {
        LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
        ColumnListView colonneCliquee = pnmv->iSubItem;

        // Si on clique sur la même colonne, on inverse le sens.
        if (g_ColonneTriee == colonneCliquee)
        {
            g_TriCroissant = !g_TriCroissant;
        }
        // Si on change de colonne, on repasse par défaut en mode croissant.
        else
        {
            g_ColonneTriee = colonneCliquee;
            g_TriCroissant = TRUE;
        }

        switch (colonneCliquee) {
        case 0:
            qsort(list_view_data, list_view_data_count, sizeof(ElementFile), ComparerNom);
            break;
        case 1:
            qsort(list_view_data, list_view_data_count, sizeof(ElementFile), ComparerTaille);
            break;
        }

        // Rafraîchit la plage de lignes
        SendMessage(list_view_hwnd, LVM_REDRAWITEMS, 0, list_view_data_count - 1);

        InvalidateRect(list_view_hwnd, NULL, TRUE);
        UpdateWindow(list_view_hwnd);

        return TRUE;
    }
    return FALSE;
}

static BOOL List_View_Msg_GetInfo(LPARAM lParam)
{
    NMHDR *plvdi = (NMHDR *)lParam;

    if (plvdi->idFrom == IDC_MY_LISTVIEW && plvdi->code == LVN_GETDISPINFOW)
    {
        NMLVDISPINFOW *plvdi = (NMLVDISPINFOW *)lParam;
        int index = plvdi->item.iItem; // Le numéro de la ligne demandée

        if (index < list_view_data_count)
        {
            // Donner l'icone correspondant
            if (plvdi->item.mask & LVIF_IMAGE)
            {
                plvdi->item.iImage = list_view_data[index].indexIcon;
            }
            // le contrôle demande le texte de la colonne
            if (plvdi->item.mask & LVIF_TEXT)
            {
                if (plvdi->item.iSubItem == 0)
                {
                    plvdi->item.pszText = list_view_data[index].name;
                    return TRUE;
                }
                else if (plvdi->item.iSubItem == 1)
                {
                    if (list_view_data[index].flags & ElementSystem)
                    {
                        plvdi->item.pszText = L"SystemFile";
                    }
                    else
                    {
                        ULONGLONG bytesSize = list_view_data[index].size;
                        StrFormatByteSizeW((ULONGLONG)bytesSize, plvdi->item.pszText, plvdi->item.cchTextMax);
                    }
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

void UpdateListView()
{
    SendMessageW(list_view_hwnd, LVM_SETITEMCOUNT, (WPARAM)list_view_data_count, 0);
    InvalidateRect(list_view_hwnd, NULL, TRUE);
    qsort(list_view_data, list_view_data_count, sizeof(ElementFile), ComparerNom);
}

BOOL NotifListView(LPARAM lParam)
{
    BOOL r = List_View_Msg_ClickHeader(lParam);

    r |= List_View_Msg_GetInfo(lParam);

    return r;
}