#pragma once
#include "feature.h"

/*
Role :	Ce fichier va contenir les evenements des boutons lors
		d'un click gauche
*/

typedef const void* (*Callback) (void);

const ListFile* LaunchChoiceExplorer(void);
const ListFile* OpenDialogFolder(void);