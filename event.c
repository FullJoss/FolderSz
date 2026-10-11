#include "pch.h"
#include "event.h"
#include "feature.h"
#include "list_view.h"
#include "dialog.h"
#include "choice_explorer.h"

static wchar_t g_current_path[MAX_PATH] = { 0 };

void OpenDialogFolder(void)
{
    wchar_t *chaine = load_folder();

    if (chaine == NULL) return;

    scan_filepath(chaine);

    UpdateListView();

    free(chaine);
}

void LaunchChoiceExplorer(void) {
    if (create_choice_explorer(mainHwnd, g_current_path, MAX_PATH))
    {
        scan_filepath(g_current_path);
        UpdateListView();
    }
    else
        MessageBox_View(L"Aucune explorer windows ouvert");
}