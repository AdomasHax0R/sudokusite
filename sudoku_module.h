// sudoku_module.h - simple sudoku generator/solver + html exporter

// goal: keep it small and no external libs
//generates a full valid Sudoku solution via backtracking
//creates a playable puzzle by removing numbers (difficulty = number of holes)
//provides basic validation helpers
//exports a static HTML page (pure HTML/CSS) with the puzzle pre-filled



#ifndef SUDOKU_MODULE_H
#define SUDOKU_MODULE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SUDOKU_SIZE 9

typedef struct SudokuBoard {
    // 0 = empty cell, 1..9 = value
    int cell[SUDOKU_SIZE][SUDOKU_SIZE];
} SudokuBoard;

typedef enum SudokuDifficulty {
    SUDOKU_DIFFICULTY_EASY = 0,
    SUDOKU_DIFFICULTY_MEDIUM = 1,
    SUDOKU_DIFFICULTY_HARD = 2
} SudokuDifficulty;

typedef enum SudokuResult {
    SUDOKU_OK = 0,
    SUDOKU_ERR_INVALID_ARG = 1,
    SUDOKU_ERR_UNSOLVABLE = 2,
    SUDOKU_ERR_IO = 3
} SudokuResult;

typedef struct SudokuTheme {
    //simple theming (used only in generated css overrides inside the htmL)
    //strings should be valid css values, eg "#dabfae" or "rgb(153, 11, 58)"
    const char* panel_bg;
    const char* cell_hover_bg;
    const char* page_title;
} SudokuTheme;

//randomness
//seeds the internal rng used for shuffling and puzzle generation
//iff you never call this, it will behave deterministically
void sudoku_seed(unsigned int seed);

// board helpers
void sudoku_clear(SudokuBoard* board);
void sudoku_copy(SudokuBoard* dst, const SudokuBoard* src);

//returns 1 if board has no rule violations (ignores 0), otherwise 0
int sudoku_is_valid_partial(const SudokuBoard* board);

//returns 1 if placing `value` at (row, col) is valid, otherwise 0
//row/col are 0..8, value is 1..9
int sudoku_can_place(const SudokuBoard* board, int row, int col, int value);

//solver/generator
//solves a puzzle in-place (0 = empty); returns SUDOKU_OK if solved
SudokuResult sudoku_solve(SudokuBoard* in_out_board);

//generates a full solved board
SudokuResult sudoku_generate_solution(SudokuBoard* out_solution);

//generates a puzzle+solution pair.
//the puzzle will contain 0 in empty cells; solution will be fully filled
SudokuResult sudoku_generate_puzzle(
    SudokuBoard* out_puzzle,
    SudokuBoard* out_solution,
    SudokuDifficulty difficulty
);

//html exporter
//writes an html page

// (header + main with .game and 81 .cell divs); filled cells get class "given"
//poarams:
//html_path: where to write the html file (eg "sudoku.html")
//css_href: what to put into <link href=" "> (efg "style.css")
//puzzle: the sudoku puzzle to render (0 cells become empty
//theme: optional (can be null); if set, adds a small <style> override block for colors and title

SudokuResult sudoku_write_html_page(
    const char* html_path,
    const char* css_href,
    const SudokuBoard* puzzle,
    const SudokuTheme* theme,
    SudokuDifficulty difficulty
);

//writes a page like sudoku_write_html_page(), but can also embed
//<script src="sudoku.js" defer></script>
//data-solution=" " attribute (81 digits) for simple validation in browser

//if out_solution is null, the page is still playable but the browser cannot validate mistakes
SudokuResult sudoku_write_html_page_with_solution(
    const char* html_path,
    const char* css_href,
    const SudokuBoard* puzzle,
    const SudokuBoard* out_solution,
    const SudokuTheme* theme,
    SudokuDifficulty difficulty
);

//utility: difficulty -> number of holes cell=0
int sudoku_holes_for_difficulty(SudokuDifficulty difficulty);

#ifdef __cplusplus
}
#endif

#endif
