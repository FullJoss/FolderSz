#include "pch.h"
#include "event.h"
#include "list_view.h"
#include "dialog.h"
#include "choice_explorer.h"

static wchar_t m_current_path[MAX_PATH] = { 0 };

const ListFile* OpenDialogFolder(void)
{
    wchar_t *chaine = LoadFolderDialog();

    if (chaine == NULL) return NULL;

    const ListFile* lf = ScanFilePath(chaine);

    free(chaine);

    return lf;
}

const ListFile* LaunchChoiceExplorer(void) {
    if (CreateChoiceExplorer(mainHwnd, m_current_path, MAX_PATH))
    {
        return ScanFilePath(m_current_path);
    }
    else
        MessageBox_View(L"No Windows Explorer windows are open.");

    return NULL;
}