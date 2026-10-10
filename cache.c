#include "cache.h"
#include <stdio.h>
#include "dialog.h"
#include <string.h>

static int g_NbCacheEntries = 0;
static CacheEntry* g_CacheTable = NULL;

static wchar_t g_PathBuffer[32768];

static unsigned int iteration_tracker = 0;

void LoadCache(void)
{
    g_NbCacheEntries = 0;

    g_CacheTable = (CacheEntry*)calloc(MAX_CACHE_ENTRIES, sizeof(CacheEntry));
    if (g_CacheTable == NULL)
        return;

    FILE *f = NULL;
    // On tente d'ouvrir le fichier en lecture binaire
    if (_wfopen_s(&f, L"cache.dat", L"rb") == 0 && f != NULL)
    {
        g_NbCacheEntries = (int)fread(g_CacheTable, sizeof(CacheEntry), MAX_CACHE_ENTRIES, f);
        fclose(f);
    }
}

void SaveCache(void)
{
    FILE *f = NULL;
    if (_wfopen_s(&f, L"cache.dat", L"wb") == 0 && f != NULL)
    {
        fwrite(g_CacheTable, sizeof(CacheEntry), MAX_CACHE_ENTRIES, f);
        fclose(f);
    }
}

CacheStatus FindSizeCachePath(ULONGLONG hash, FILETIME dateActuelle, ULONGLONG *outTaille)
{
    CacheStatus res = CACHE_STATUS_UNDEFINED;

    size_t index = (size_t)(hash % MAX_CACHE_ENTRIES);
    size_t begin = index;
    while (g_CacheTable[index].use && g_CacheTable[index].hash != hash) {
        index = (index + 1) % MAX_CACHE_ENTRIES;

        if (begin == index) {
            MessageBox_View(L"Erreur critique : La table de hachage est pleine !");
            return CACHE_STATUS_ERROR;
        }
    }

    if (g_CacheTable[index].use) {
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

            // 3. On écrit le sous-élément. g_PathBuffer devient "D:\Dossier\SousElement"
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
                    // C'est un fichier basique : lecture instantanée de sa taille
                    ULONGLONG tailleFichier = ((ULONGLONG)pFindData->nFileSizeHigh << 32) | pFindData->nFileSizeLow;
                    tailleTotale += tailleFichier;
                }

                g_PathBuffer[ancienneLongueur] = L'\0';
            }

        } while (FindNextFileW(hFind, pFindData));

        FindClose(hFind);
        free(pFindData);

        UpdateCacheElement(hashPath, tailleTotale, lastModif);
    }

    return tailleTotale;
}

void UpdateCacheElement(ULONGLONG hash, ULONGLONG size, FILETIME time) {

    // 1. Calcul de l'index de départ avec le modulo
    size_t index = (size_t)(hash % MAX_CACHE_ENTRIES);
    size_t depart = index;

    while (g_CacheTable[index].use && g_CacheTable[index].hash != hash) {
        index = (index + 1) % MAX_CACHE_ENTRIES;

        if (index == depart) {
            MessageBox_View(L"Erreur critique : La table de hachage est pleine !");
            return;
        }
    }

    if (!g_CacheTable[index].use) {
        // Insert
        g_CacheTable[index].hash = hash;
        g_CacheTable[index].use = TRUE;
    }

    // Update
    g_CacheTable[index].size = size;
    g_CacheTable[index].derniereModif = time;
}

ULONGLONG fnv_1a(const wchar_t *chemin)
{
    // Constantes officielles de l'algorithme FNV-1a pour 64 bits
    ULONGLONG hash = 0xcbf29ce484222325ULL;
    const ULONGLONG prime = 0x100000001b3ULL;

    // parcourt la chaîne
    for (size_t i = 0; chemin[i] != L'\0'; ++i)
    {
        // Extraction du premier octet du caractère (UNICODE)
        UINT8 octet1 = (UINT8)(chemin[i] & 0xFF);
        hash ^= octet1;
        hash *= prime;

        // Extraction du second octet du caractère
        UINT8 octet2 = (UINT8)((chemin[i] >> 8) & 0xFF);
        hash ^= octet2;
        hash *= prime;
    }

    return hash;
}