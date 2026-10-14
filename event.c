#include "pch.h"
#include "event.h"
#include "feature.h"
#include "list_view.h"
#include "dialog.h"
#include "choice_explorer.h"

static wchar_t m_current_path[MAX_PATH] = { 0 };

void OpenDialogFolder(void)
{
    wchar_t *chaine = LoadFolderDialog();

    if (chaine == NULL) return;

    ListFile* lf = ScanFilePath(chaine);
    UpdateListView(lf);

    free(chaine);
}

void LaunchChoiceExplorer(void) {

    if (CreateChoiceExplorer(mainHwnd, m_current_path, MAX_PATH))
    {
        ListFile* lf = ScanFilePath(m_current_path);
        UpdateListView(lf);
    }
    else
        MessageBox_View(L"No Windows Explorer windows are open.");
}