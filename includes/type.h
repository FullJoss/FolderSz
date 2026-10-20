/*
 * FolderSz - Folder Size Utility
 * Copyright (c) 2026 Joss HR
 * Licensed under the MIT License.
 * See LICENSE file in the project root for details.
 */


#pragma once
#include "pch.h"

typedef void (*Callback) (HWND, void*);
typedef int (*CallbackSort) (const void*, const void*, BOOL isAscending);

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
    int width;
    CallbackSort FuncComparison;
} ColumnListView;

typedef enum
{
    ElementInvalid = 0,      
    ElementDirectory = 1 << 0,
    ElementSystem = 1 << 1,
    ElementPartition = 1 << 2,
    ElementCache = 1 << 3,
    ElementParentDirectory = 1 << 4,
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
    RECT headRect;

    HRGN clipRgn;
    ColumnListView* columns;
    void* rows;
    size_t row_size;
    
    BOOL isDraggingScroll;
    BOOL enableScroll;
    BOOL ascendingSort;
    
    int scrollPos;
    int maxScrollPos;
    int dragYOffset;
    int indexRowHover;
    int indexColHover;
    int indexColSort;
    int indexRowSelect;
    int sbThumbHeight;
    int maxThumbY;
    unsigned int n_columns;
    unsigned int n_rows;

    void (*OnDoubleClickRow) (HWND, void*);
} ListFileView;

typedef struct
{
    Button* buttons;
    ListFileView* lfv;
    int n_buttons;
    int width;
    int height;
} WindowDataInformation;