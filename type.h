#pragma once
#include "pch.h"

typedef void (*Callback) (HWND, void*);

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
    void* item;
} RowListView;

typedef struct {
    RECT padding;
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
    ElementFile* files;
    unsigned int count;
    unsigned int capacity;
} ListFile;

typedef struct {
    RECT bounds;
    RECT itemsArea;
    RECT sbTrack;

    HRGN clipRgn;
    ColumnListView* columns;
    void* rows;
    size_t row_size;
    
    BOOL isDraggingScroll;
    
    int scrollPos;
    int dragYOffset;
    int indexHover;
    int sbThumbHeight;
    int n_columns;
    int n_rows;
} ListFileView;

typedef struct
{
    Button* buttons;
    int n_buttons;
    ListFileView* lfv;
} WindowDataInformation;