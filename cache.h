#include <windows.h>

#define MAX_CACHE_ENTRIES 312083

typedef struct
{
    ULONGLONG hash;
    ULONGLONG size;
    FILETIME derniereModif;
    BOOL use;
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

ULONGLONG SearchFolderSize(const wchar_t *dossierPath, FILETIME lastModif);
ULONGLONG GetSizeCachePath(size_t lenPath, FILETIME lastModif);

void SaveCache(void);
void LoadCache(void);

ULONGLONG fnv_1a(const wchar_t *chemin);