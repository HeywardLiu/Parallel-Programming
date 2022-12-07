#include "sudoku.h"
#include "omp.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include "CycleTimer.h"

using namespace std;

void crook_pruning(Sudoku *sudoku);
int crooks_solver(Sudoku *sudoku);

static const int threads_in_parallel = 16;  // Specify omp parallel threads
static const int filled_cells_number = 2;   // fill first K unknown cells

typedef struct
{
    int row;
    int col;
    vector<int> possible_values;
} UnknownCell;

// Show possible values in each unknonws Cell (first K unknown cell)
void show_unknownCell_list(vector<UnknownCell> &unknown_cell_list) 
{
    cout << "[ Possible value of unknown cells ]" << endl;
    for (int i = 0; i < unknown_cell_list.size(); i++) 
    {
        int row = unknown_cell_list[i].row; 
        int col = unknown_cell_list[i].col;
        cout << "Cell(" << row << ", " << col << ") = ";
        for(int j = 0; j <unknown_cell_list[i].possible_values.size(); j++) 
        {
            cout << unknown_cell_list[i].possible_values[j] << " ";
        }
        cout << endl;
    }
    cout << endl << endl;
}


// get first K unknown cells' cordinate 
int get_UnknownCell_list(vector<UnknownCell> &cell_list, const Markup *markup, const int num) 
{
    int row = 0, col = 0, total_threads = 1;
    while(cell_list.size() < num) 
    {
        // find unknown cell
        if ((*markup)[row][col]) 
        {  
            UnknownCell cell;
            cell.row = row;
            cell.col = col;
            
            // push all possible value into a list
            for (int v = 1; v <= N; v++)
            {
                if (((*markup)[row][col] & (1 << (v-1)))) 
                {
                    cell.possible_values.push_back(v);
                }
            }
            cell_list.push_back(cell);
            total_threads *= cell.possible_values.size();
        }
        row = row + (col == N-1);
        col = (col + 1)%N;
    }
    return total_threads;  // number of total threads to be vaildated
}

// Find total threads to be validated 
// Total threads = permutation of possible values in the first K unknown cells.
void get_perm(vector<Sudoku*> &sudoku_list, Sudoku *sudoku, const vector<UnknownCell> &cell_list, const int depth) 
{
    UnknownCell cell = cell_list[depth];  
    for(int i = 0; i < cell.possible_values.size(); i++) 
    {
        Cell temp_cell;
        temp_cell.row = cell.row;
        temp_cell.col = cell.col;
        temp_cell.val = cell.possible_values[i];
        heap_push(&sudoku->heap, &temp_cell, &sudoku->markup, &sudoku->grid);
        set_value(sudoku, temp_cell.row, temp_cell.col, temp_cell.val);

        if(depth < cell_list.size()-1) 
        {
            get_perm(sudoku_list, sudoku, cell_list, depth+1);
        } 
        else if (depth == cell_list.size()-1) 
        {
            Sudoku* copy = new Sudoku;
            copy_sudoku(copy, sudoku);
            sudoku_list.push_back(copy);
        }
        heap_pop(&sudoku->heap, &temp_cell, &sudoku->markup, &sudoku->grid);
    }
}

void show_sudoku_list(const vector<Sudoku*> sudoku_list) 
{
    cout << "[ Show parallel sudoku list ]" << endl;
    for(int i = 0; i < sudoku_list.size(); i++) 
    {
        show_grid(&sudoku_list[i]->grid);
    }
}

// Return number of unknown cells in a sudoku
int count_unknown(Sudoku *sudoku) 
{
    int count = 0;
    for(int i = 0; i < N; i++) 
    {
        for(int j = 0; j < N; j++)
        {
            if((sudoku->grid)[i][j] == UNASSIGNED)
            {
                count++;
            }
        }
    }
    return count;
}

int solve(Sudoku *sudoku) 
{
    double serial_start_time = 0, serial_end_time = 0, serial_elapsed_time = 0;
    cout << "[ Parallelized Crook's solver ]" << endl;
    serial_start_time  = CycleTimer::currentSeconds();

    // Crook pruning
    crook_pruning(sudoku);
    int unknown = count_unknown(sudoku);
    cout << "[ Crook pruning ]" << endl << " # Unknowns cells in the grid after pruning: " << unknown << endl;
    cout << "---------------------------" << endl << endl;
    if(unknown==0) 
    {
        serial_end_time = CycleTimer::currentSeconds();
        serial_elapsed_time = serial_end_time - serial_start_time;
        if(validate_solution(&sudoku->grid)) {
            cout << "Problem has been solved without parallel backtracking! " << endl;
            cout << serial_elapsed_time << " sec " << endl;
            cout << "-----------------" << endl << endl;
        }
        exit(0);
    }

    Grid *grid = &sudoku->grid;
    Heap *heap = &sudoku->heap;
    Markup *markup = &sudoku->markup;
    
    // Get first K unknowns cells (including index and possible values)
    vector<UnknownCell> unknown_cell_list;
    unknown_cell_list.reserve(filled_cells_number);
    int total_threads = get_UnknownCell_list(unknown_cell_list, markup, filled_cells_number);
    show_unknownCell_list(unknown_cell_list);
    
    // Get permutations of possible values in the first K unknowm cells
    vector<Sudoku*> parallel_sudoku_list;
    parallel_sudoku_list.reserve(total_threads);   // A simple optimization to avoid memcpy
    get_perm(parallel_sudoku_list, sudoku, unknown_cell_list, 0);  
    serial_end_time  = CycleTimer::currentSeconds();
    serial_elapsed_time = serial_end_time - serial_start_time;

    // Verbose
    cout << "-- Original Grid --" << endl;
    show_grid(&sudoku->grid);
    cout << "Number of Unknown cells been filled: "  << unknown_cell_list.size() << endl;
    cout << "              Number of omp threads: " << threads_in_parallel << endl; 
    cout << "  Number of Threads to be validated: " << parallel_sudoku_list.size() << endl;
    cout << "    Time to prepare parallel sudoku: " << serial_elapsed_time << endl << endl;
    
    // Parallelized & recursive solver
    double parallel_start_time = CycleTimer::currentSeconds();
    #pragma omp parallel for num_threads (threads_in_parallel)
    for(int i = 0; i < parallel_sudoku_list.size(); i++) 
    {
        double parallel_end_time = 0;
        crook_pruning(parallel_sudoku_list[i]);
        crooks_solver(parallel_sudoku_list[i]);

        #pragma omp critical
        {
            if(validate_solution(&parallel_sudoku_list[i]->grid)) 
            {

                parallel_end_time = CycleTimer::currentSeconds();
                cout << "-- Answer found! --" << endl;
                cout << "Response time (1st answer return): " << (parallel_end_time - parallel_start_time) + serial_elapsed_time << " (sec)" << endl;
                cout << "Thread number (1st answer return): " << i << endl;
                show_grid(&parallel_sudoku_list[i]->grid);
                copy_sudoku(sudoku, parallel_sudoku_list[i]);
                exit(0);
                
            }
            else 
            {
                // cout << i << " : Wrong Answer!" << endl;
            }
        // cout << "-----------------------" << endl;
        }
    }
}

// serialized & non-recursive solver with crooks algorithm
int crooks_solver(Sudoku *sudoku)
{
    Grid *grid = &sudoku->grid;
    Heap *heap = &sudoku->heap;
    Markup *markup = &sudoku->markup;
    Cell top_cell;
    int row = 0, col = 0, val;
    int find_safe_insertion;

    // perform elimination & lone ranger first
    crook_pruning(sudoku);

    // set_value
    while (row != N)
    {
        find_safe_insertion = 0;
        if ((*grid)[row][col] == UNASSIGNED){
            if ((*markup)[row][col])
            {
                for (int v = 1; v <= N; v++)
                {
                    if ((*markup)[row][col] & (1 << (v-1)))
                    {
                        // find safe insertion, make a guess
                        Cell cell = {row, col, v};
                        heap_push(heap, &cell, &sudoku->markup, &sudoku->grid);
                        set_value(sudoku, row, col, v);
                        crook_pruning(sudoku);
                        find_safe_insertion = 1;
                        break;
                    }
                }
            }
            while (!find_safe_insertion)
            {
                if (heap->count == 0)
                    return 0;
                // fail to find safe insertion, backtrack
                // try other value for the top cell
                heap_pop(heap, &top_cell, &sudoku->markup, &sudoku->grid);  // back-tracking
                row = top_cell.row;
                col = top_cell.col;
                val = top_cell.val;
                (*grid)[row][col] = UNASSIGNED;
                for (val =  top_cell.val + 1; val <= N; val++)
                {
                    if ((*markup)[row][col] & (1 << (val-1)))
                    {
                        Cell cell = {row, col, val};
                        heap_push(heap, &cell, &sudoku->markup, &sudoku->grid);
                        set_value(sudoku, row, col, val);
                        crook_pruning(sudoku);
                        find_safe_insertion = 1;
                        break;
                    }
                }
            }
        }
        row = row + (col == N-1);
        col = (col + 1)%N;
    }
    return 1;
}

void crook_pruning(Sudoku *sudoku)
{
    int changed, e_changed, l_changed;
    do
    {
        changed = 0;
        do
        {
            e_changed = elimination(sudoku);
            changed += e_changed;
        } while (e_changed);
        do
        {
            l_changed = lone_ranger(sudoku);
            changed += l_changed;
        } while (l_changed);
    } while (changed);
}