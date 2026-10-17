/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#include "pch.h"
#include "tick.h"

LARGE_INTEGER frequence;
LARGE_INTEGER tempsInitial, tempsActuel;

void InitTick() {
    QueryPerformanceFrequency(&frequence);
    QueryPerformanceCounter(&tempsInitial);
}

double GetElapsedTime() {
    QueryPerformanceCounter(&tempsActuel);
    double deltaTime = (double)(tempsActuel.QuadPart - tempsInitial.QuadPart) / frequence.QuadPart;
    tempsInitial = tempsActuel;

    return deltaTime;
}