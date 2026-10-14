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