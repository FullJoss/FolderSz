#include "cache.h"
#include <stdio.h>
#include "dialog.h"
#include "feature.h"
#include <string.h>

#define CACHE_FILE_NAME L"FolderSz.cache"

#define SET_USE(mask, idx)   ((mask)[(idx) >> 5] |= (1 << ((idx) & 31)))
#define GET_USE(mask, idx)   ((mask)[(idx) >> 5] & (1 << ((idx) & 31)))

static unsigned int g_NbCacheEntries = 0;

static CacheEntry g_CacheTable[MAX_CACHE_ENTRIES] = { 0 };
static unsigned int g_CacheBitTable[MAX_BITMASK_ENTRIES] = { 0 };

static wchar_t g_PathBuffer[32768];

static unsigned int iteration_tracker = 0;

static inline void ReadCacheBinaryFile(wchar_t* filename);
static inline void DecompressTable(CacheEntry* buffer);
static inline CacheEntry* CompressTable();

void LoadCache(void)
{
    ReadCacheBinaryFile(CACHE_FILE_NAME);
}

void SaveCache(void)
{
    CacheEntry* bufferRAM = CompressTable();
    if (!bufferRAM) return;

    FILE *f = NULL;
    if (_wfopen_s(&f, CACHE_FILE_NAME, L"wb") == 0 && f != NULL)
    {
        fwrite(g_CacheBitTable, sizeof(unsigned int), MAX_BITMASK_ENTRIES, f);
        fwrite(&g_NbCacheEntries, sizeof(unsigned int), 1, f);
        fwrite(bufferRAM, sizeof(CacheEntry), g_NbCacheEntries, f);
        fclose(f);
    }

    free(bufferRAM);
}

CacheStatus FindSizeCachePath(ULONGLONG hash, FILETIME dateActuelle, ULONGLONG *outTaille)
{
    CacheStatus res = CACHE_STATUS_UNDEFINED;

    size_t index = (size_t)(hash % MAX_CACHE_ENTRIES);
    size_t begin = index;
    while (GET_USE(g_CacheBitTable, index) && g_CacheTable[index].hash != hash) {
        index = (index + 1) % MAX_CACHE_ENTRIES;

        if (begin == index) {
            MessageBox_View(L"Erreur critique : La table de hachage est pleine !");
            return CACHE_STATUS_ERROR;
        }
    }

    if (GET_USE(g_CacheBitTable, index)) {
        if (CompareFileTime(&g_CacheTable[index].derniereModif, &dateActuelle) == 0)
        {
            *outTaille = g_CacheTable[index].size;
            res = CACHE_STATUS_UPDATE;
            OutputDebugStringW(L"Puisser dans le cache !\n");
        }
        else {
            res = CACHE_STATUS_OLD;
        }
    }

    return res;
}

ULONGLONG SearchFolderSize(const wchar_t *dossierPath, FILETIME lastModif)
{
    size_t length = wcslen(dossierPath);
    swprintf_s(g_PathBuffer, length + 1, L"%s", dossierPath);
    return GetSizeCachePath(length, lastModif);
}

ULONGLONG GetSizeCachePath(size_t lenPath, FILETIME lastModif)
{
    ULONGLONG tailleTotale = 0;

    ULONGLONG hashPath = fnv_1a(g_PathBuffer);

    CacheStatus sresult = FindSizeCachePath(hashPath, lastModif, &tailleTotale);

    if (sresult & (CACHE_STATUS_ERROR | CACHE_STATUS_UPDATE))
        return tailleTotale;

    else
    {
        // 1. Préparation sûre du joker pour Windows
        g_PathBuffer[lenPath] = L'\\';
        g_PathBuffer[lenPath + 1] = L'*';
        g_PathBuffer[lenPath + 2] = L'\0';

        WIN32_FIND_DATAW *pFindData = (WIN32_FIND_DATAW *)malloc(sizeof(WIN32_FIND_DATAW));
        if (pFindData == NULL)
        {
            free(pFindData);
            return 0;
        }

        HANDLE hFind = FindFirstFileExW(g_PathBuffer, FindExInfoBasic, pFindData, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);

        // 2. On restaure IMMEDIATEMENT le chemin propre pour ce niveau
        g_PathBuffer[lenPath] = L'\0';

        if (hFind == INVALID_HANDLE_VALUE)
        {
            free(pFindData);
            return 0;
        }

        do
        {
            if (wcscmp(pFindData->cFileName, L".") == 0 || wcscmp(pFindData->cFileName, L"..") == 0)
                continue;
            if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                continue;

            size_t ancienneLongueur = lenPath;

            int ecrit = swprintf_s(g_PathBuffer + lenPath, 32768 - lenPath, L"\\%s", pFindData->cFileName);

            if (ecrit != -1)
            {
                if (pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    ULONGLONG tailleSousDossier = GetSizeCachePath(lenPath + ecrit, pFindData->ftLastWriteTime);
                    tailleTotale += tailleSousDossier;
                }
                else
                {
                    ULONGLONG tailleFichier = ((ULONGLONG)pFindData->nFileSizeHigh << 32) | pFindData->nFileSizeLow;
                    tailleTotale += tailleFichier;
                }

                g_PathBuffer[ancienneLongueur] = L'\0';
            }

            RefreshView();
        } while (FindNextFileW(hFind, pFindData));

        FindClose(hFind);
        free(pFindData);

        UpdateCacheElement(hashPath, tailleTotale, lastModif);
    }

    return tailleTotale;
}

void UpdateCacheElement(ULONGLONG hash, ULONGLONG size, FILETIME time) {
    size_t index = (size_t)(hash % MAX_CACHE_ENTRIES);
    size_t depart = index;

    while (GET_USE(g_CacheBitTable, index) && g_CacheTable[index].hash != hash) {
        index = (index + 1) % MAX_CACHE_ENTRIES;

        if (index == depart) {
            MessageBox_View(L"Erreur critique : La table de hachage est pleine !");
            return;
        }
    }

    if (!GET_USE(g_CacheBitTable, index)) {
        // Insert
        g_CacheTable[index].hash = hash;
        SET_USE(g_CacheBitTable, index);
        g_NbCacheEntries++;
    }

    // Update
    g_CacheTable[index].size = size;
    g_CacheTable[index].derniereModif = time;
}

ULONGLONG fnv_1a(const wchar_t *chemin)
{
    ULONGLONG hash = 0xcbf29ce484222325ULL;
    const ULONGLONG prime = 0x100000001b3ULL;

    for (size_t i = 0; chemin[i] != L'\0'; ++i)
    {
        // two bytes (UNICODE)
        UINT8 byte1 = (UINT8)(chemin[i] & 0xFF);
        hash ^= byte1;
        hash *= prime;

        UINT8 byte2 = (UINT8)((chemin[i] >> 8) & 0xFF);
        hash ^= byte2;
        hash *= prime;
    }

    return hash;
}

static inline void ReadCacheBinaryFile(wchar_t* filename) {

    CacheEntry* bufferRAM = NULL;

    FILE* f = NULL;

    if (_wfopen_s(&f, filename, L"rb") == 0 && f != NULL)
    {
        size_t count = fread(g_CacheBitTable, sizeof(unsigned int), MAX_BITMASK_ENTRIES, f);
        
        if (count != MAX_BITMASK_ENTRIES) {
            perror("Error opening cache");
            fclose(f);
        }

        fread(&g_NbCacheEntries, sizeof(unsigned int), 1, f);

        if (g_NbCacheEntries > 0) {
            bufferRAM = malloc(sizeof(CacheEntry) * g_NbCacheEntries);
            if (!bufferRAM) {
                perror("Save Error Buffer Allocation");
                fclose(f);
                return;
            }

            fread(bufferRAM, sizeof(CacheEntry), g_NbCacheEntries, f);
        }

        fclose(f);
    }

    DecompressTable(bufferRAM);
}


static inline void DecompressTable(CacheEntry* buffer) {
    unsigned int ibuffer = 0;

    for (int i = 0; i < MAX_CACHE_ENTRIES; i++) {
        if (GET_USE(g_CacheBitTable, i)) {
            g_CacheTable[i] = buffer[ibuffer];

            ibuffer++;

            if (ibuffer >= g_NbCacheEntries)
                break;
        }
    }

    free(buffer);
}

static inline CacheEntry* CompressTable() {
    CacheEntry* bufferRAM = malloc(sizeof(CacheEntry) * g_NbCacheEntries);
    if (!bufferRAM) {
        perror("Save Error Buffer Allocation");
        return NULL;
    }

    unsigned int ibuffer = 0;
    for (int i = 0; i < MAX_CACHE_ENTRIES; i++) {
        if (GET_USE(g_CacheBitTable, i)) {
            bufferRAM[ibuffer] = g_CacheTable[i];

            ibuffer++;

            if (ibuffer >= g_NbCacheEntries)
                break;
        }
    }

    return bufferRAM;
}
