#pragma once

#include "pch.h"
#include "type.h"

#define VecUpdateCapacity(v, new_cap) do {                                    \
    if ((new_cap) >= (v)->capacity) {                                         \
        void* tmp = realloc((v)->data, (new_cap) * sizeof(*(v)->data));       \
        if (!tmp) {                                                           \
            perror("Erreur realloc dans VecGrow");                            \
            abort();                                                          \
        }                                                                     \
        (v)->data = tmp;                                                      \
        (v)->capacity = (new_cap);                                            \
    }                                                                         \
} while (0)

#define VecGrow(v) do {                                                       \
    if ((v)->count >= (v)->capacity) {                                        \
        size_t new_cap = (v)->capacity ? (v)->capacity * 2 : 16;              \
        VecUpdateCapacity(v, new_cap);                                        \
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
