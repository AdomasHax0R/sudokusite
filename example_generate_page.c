// example_generate_page.c
// Build:
//   gcc -std=c99 -O2 -Wall -Wextra -pedantic sudoku_module.c example_generate_page.c -o gen_page
//
// Run:
//   ./gen_page
//
// Output:
//   generated_sudoku.html (references your existing style.css)

#include "sudoku_module.h"

#include <stdio.h>
#include <time.h>

int main(void) {
    SudokuBoard puzzle;
    SudokuBoard solution;

    sudoku_seed((unsigned int)time(NULL));

    SudokuDifficulty difficulty = SUDOKU_DIFFICULTY_MEDIUM;
    SudokuResult r = sudoku_generate_puzzle(&puzzle, &solution, difficulty);
    if (r != SUDOKU_OK) {
        fprintf(stderr, "Failed to generate puzzle (error=%d)\n", (int)r);
        return 1;
    }

    SudokuTheme theme;
    theme.panel_bg = "#dabfae";           // same as your current CSS
    theme.cell_hover_bg = "wheat";        // simple hover override
    theme.page_title = "Sudoku (Generated)";

    r = sudoku_write_html_page_with_solution(
        "generated_sudoku.html",
        "style.css",
        &puzzle,
        &solution,
        &theme,
        difficulty
    );
    if (r != SUDOKU_OK) {
        fprintf(stderr, "Failed to write HTML (error=%d)\n", (int)r);
        return 1;
    }

    printf("OK: wrote generated_sudoku.html (%d holes)\n",
           sudoku_holes_for_difficulty(difficulty));
    return 0;
}

