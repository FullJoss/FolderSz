#include "pch.h"
#include "event.h"
#include "feature.h"
#include "list_view.h"
#include "dialog.h"

void OpenDialogFolder(void)
{
    wchar_t *chaine = load_folder();

    if (chaine == NULL) return;

    scan_filepath(chaine);

    List_View_MAJ();

    free(chaine);
}