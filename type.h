#pragma once
#include "pch.h"

typedef void (*Callback) (void*);

typedef struct
{
    RECT bounds;
    int state;
    const wchar_t* text;
    Callback on_click;
} Button;

typedef struct {
    int x, y;
    wchar_t text[MAX_PATH];
} Label;

typedef struct
{
    wchar_t chemin[MAX_PATH];
    HWND hwndExplorateur;
    HTHUMBNAIL hThumbnail;
    HWND hwndPlaceholder;
} ExplorerInfo;

typedef struct {
    const wchar_t* name;
    unsigned int width;
    BOOL isSorted;
    int (*FuncComparison) (const void*, const void*);
} ColumnListView;

typedef enum
{
    ElementInvalid = 0,      
    ElementDirectory = 1 << 0,
    ElementSystem = 1 << 1,
    ElementPartition = 1 << 2,
    ElementCache = 1 << 3
} ElementFlags;

typedef struct
{
    wchar_t name[MAX_PATH];
    ULONGLONG size;
    ElementFlags flags;
    int indexIcon;
} ElementFile;

typedef struct {
    ElementFile* file;
    unsigned int count;
    unsigned int capacity;
} ListFile;

typedef struct {
    RECT bounds;
    HWND hwnd;
    ListFile* data;
    int scrollPos;
    int dragYOffset;
    int indexHover;
    BOOL isDraggingScroll;

    RECT itemsArea;
    HRGN clipRgn;
    RECT sbTrack;
    int sbThumbHeight;

    ColumnListView* columns;
    int n_columns;
} ListFileView;

typedef struct
{
    Button* buttons;
    int n_buttons;
    ListFileView* lfv;
} WindowDataInformation;