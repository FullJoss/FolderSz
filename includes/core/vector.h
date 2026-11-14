#pragma once

#include "pch.h"
#include "type.h"

// La macro à utiliser directement dans vos typedef !
#define Vec(T) struct { \
    T*     data;        \
    size_t count;       \
    size_t capacity;    \
}

typedef Vec(ElementFile)  FileVec;
typedef Vec(ElementFile*) FilePtrVec;
typedef Vec(int)          IntVec;

#define VecGrow(v) do {                                                       \
    if ((v)->count >= (v)->capacity) {                                        \
        size_t new_cap = (v)->capacity ? (v)->capacity * 2 : 16;              \
        void* tmp = realloc((v)->data, new_cap * sizeof(*(v)->data));         \
        if (!tmp) {                                                           \
            perror("Erreur realloc dans VecGrow");                            \
            abort();                                                          \
        }                                                                     \
        (v)->data = tmp;                                                      \
        (v)->capacity = new_cap;                                              \
    }                                                                         \
    (v)->count++;                                                             \
} while (0)

#define VecLast(v) (&(v)->data[(v)->count - 1])

#define VecFree(v) do { \
    free((v)->data);    \
    (v)->data = NULL;   \
    (v)->count = 0;     \
    (v)->capacity = 0;  \
} while (0)
