#include "type.h"
#include <windows.h>

#define MAX_CACHE_ENTRIES 312083
#define MAX_BITMASK_ENTRIES ((MAX_CACHE_ENTRIES + 31) / 32)

typedef struct
{
    ULONGLONG hash;
    ULONGLONG size;
    FILETIME derniereModif;
} CacheEntry;

typedef enum
{
    CACHE_STATUS_UNDEFINED,
    CACHE_STATUS_UPDATE,
    CACHE_STATUS_OLD,
    CACHE_STATUS_ERROR
} CacheStatus;

void UpdateCacheElement(ULONGLONG hash, ULONGLONG size, FILETIME time);

CacheStatus FindSizeCachePath(ULONGLONG hash, FILETIME dateActuelle, ULONGLONG *outTaille);

ULONGLONG SearchFolderSize(const wchar_t *dossierPath, FILETIME lastModif, ListFileView* lfv);
ULONGLONG GetSizeCachePath(size_t lenPath, FILETIME lastModif, ListFileView* lfv);

void SaveCache(void);
void LoadCache(void);

ULONGLONG fnv_1a(const wchar_t *chemin);