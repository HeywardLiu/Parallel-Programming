#ifndef __SUDOKU_H__
#define __SUDOKU_H__

#include <string.h>
#include <stdio.h>

#define SUB_N 5
#define N ((SUB_N) * (SUB_N))
#define UNASSIGNED 0

typedef int Grid[N][N];
typedef int Markup[N][N]; // 1 for possible to fill in, 0 for not possible to fill in
typedef struct
{
    int row;
    int col;
    int val;
    // Markup orig_markup;
} Cell;
typedef struct sudoku
{
    Cell cell;
    Grid orig_grid;
    Markup orig_markup;
} Screenshot;


typedef struct
{
    Screenshot cell_arr[N * N];
    int count;
} Heap; // a heap containing the inserted cells for a given sudoku puzzle

typedef struct
{
    Grid grid;
    Heap heap;
    Markup markup;
} Sudoku;

inline int markup_contain(Markup *markup, int row, int col, int val){ return ((*markup)[row][col] >> (val-1)) & 1; }
inline void remove_from_markup(Markup *markup, int row, int col, int val){ (*markup)[row][col] &= ~(1 << (val-1)); }
// or return whether or not the original bit is set

inline void heap_push(Heap *heap, Cell *cell, Markup *markup, Grid *grid){
    heap->cell_arr[heap->count].cell = *cell;
    memcpy(&heap->cell_arr[heap->count].orig_markup, markup, sizeof(Markup));
    memcpy(&heap->cell_arr[heap->count].orig_grid, grid, sizeof(Grid));
    heap->count++;
}
inline void heap_pop(Heap *heap, Cell *cell, Markup *markup, Grid *grid){
    heap->count--;
    *cell = heap->cell_arr[heap->count].cell;
    memcpy(markup, &heap->cell_arr[heap->count].orig_markup, sizeof(Markup));
    memcpy(grid, &heap->cell_arr[heap->count].orig_grid, sizeof(Grid));
}

void set_value(Sudoku *sudoku, int row, int col, int val);

// General, defined in sudoku.cpp
void show_grid(const Grid *grid);
void show_heap(const Heap *heap);
void show_markup(const Markup *markup);
void show_sudoku(const Sudoku *sudoku);
int is_safe(const Grid *grid, int row, int col, int num);
int validate_solution(const Grid *grid);
void sudoku_reset(Sudoku *sudoku);
// crook's algorithm: elimination, lone ranger, twins and triplets
int elimination(Sudoku *sudoku);
int lone_ranger(Sudoku *sudoku);

// defined for each solver
extern int solve(Sudoku *sudoku);

#endif //__SUDOKU_H__