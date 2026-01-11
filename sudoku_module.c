// sudoku_module.c - implementation

#include "sudoku_module.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

// rng (simple wrapper around rand)

static int g_seeded = 0;

void sudoku_seed(unsigned int seed) {
    srand(seed);
    g_seeded = 1;
}

static void seed_if_needed(void) {
    if (!g_seeded) {
        sudoku_seed(0u);
    }
}

static int rand_int(int max_exclusive) {
    //max_exclusive must be > 0
    return rand() % max_exclusive;
}

static void shuffle_ints(int* a, int n) {
    //naudojam Fisher-Yates shuffle
    for (int i = n - 1; i > 0; --i) {
        int j = rand_int(i + 1);
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

//board helpers
void sudoku_clear(SudokuBoard* board) {
    if (!board) return;
    for (int r = 0; r < SUDOKU_SIZE; ++r) {
        for (int c = 0; c < SUDOKU_SIZE; ++c) {
            board->cell[r][c] = 0;
        }
    }
}

void sudoku_copy(SudokuBoard* dst, const SudokuBoard* src) {
    if (!dst || !src) return;
    memcpy(dst->cell, src->cell, sizeof(dst->cell));
}

static int is_in_range_1_9(int v) {
    return v >= 1 && v <= 9;
}

int sudoku_can_place(const SudokuBoard* board, int row, int col, int value) {
    if (!board) return 0;
    if (row < 0 || row >= 9 || col < 0 || col >= 9) return 0;
    if (!is_in_range_1_9(value)) return 0;

    //row / col checks
    for (int i = 0; i < 9; ++i) {
        if (board->cell[row][i] == value) return 0;
        if (board->cell[i][col] == value) return 0;
    }

    //3x3 box check
    int br = (row / 3) * 3;
    int bc = (col / 3) * 3;
    for (int r = br; r < br + 3; ++r) {
        for (int c = bc; c < bc + 3; ++c) {
            if (board->cell[r][c] == value) return 0;
        }
    }

    return 1;
}

int sudoku_is_valid_partial(const SudokuBoard* board) {
    if (!board) return 0;

    //check that every non 0 value is placeable (ignoring itself)
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            int v = board->cell[r][c];
            if (v == 0) continue;
            if (!is_in_range_1_9(v)) return 0;

            SudokuBoard tmp = *board;
            tmp.cell[r][c] = 0;
            if (!sudoku_can_place(&tmp, r, c, v)) return 0;
        }
    }
    return 1;
}

//solver (simple backtracking)
static int find_empty_cell(const SudokuBoard* b, int* out_r, int* out_c) {
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (b->cell[r][c] == 0) {
                *out_r = r;
                *out_c = c;
                return 1;
            }
        }
    }
    return 0;
}

static int solve_backtrack(SudokuBoard* b) {
    int row = 0, col = 0;
    if (!find_empty_cell(b, &row, &col)) {
        return 1; // solved
    }

    int nums[9];
    for (int i = 0; i < 9; ++i) nums[i] = i + 1;
    shuffle_ints(nums, 9);

    for (int i = 0; i < 9; ++i) {
        int v = nums[i];
        if (sudoku_can_place(b, row, col, v)) {
            b->cell[row][col] = v;
            if (solve_backtrack(b)) return 1;
            b->cell[row][col] = 0;
        }
    }
    return 0;
}

SudokuResult sudoku_solve(SudokuBoard* in_out_board) {
    if (!in_out_board) return SUDOKU_ERR_INVALID_ARG;
    seed_if_needed();
    if (!sudoku_is_valid_partial(in_out_board)) return SUDOKU_ERR_UNSOLVABLE;
    if (solve_backtrack(in_out_board)) return SUDOKU_OK;
    return SUDOKU_ERR_UNSOLVABLE;
}

SudokuResult sudoku_generate_solution(SudokuBoard* out_solution) {
    if (!out_solution) return SUDOKU_ERR_INVALID_ARG;
    seed_if_needed();
    sudoku_clear(out_solution);
    if (solve_backtrack(out_solution)) return SUDOKU_OK;
    return SUDOKU_ERR_UNSOLVABLE;
}

//count holes (0)
static int count_holes(const SudokuBoard* b) {
    int holes = 0;
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (b->cell[r][c] == 0) ++holes;
        }
    }
    return holes;
}

int sudoku_holes_for_difficulty(SudokuDifficulty difficulty) {
    //simple mapping:
    //easy: fewer holes (more given numbers), medium eg. something itn the middle
    //hard: more holes
    switch (difficulty) {
        case SUDOKU_DIFFICULTY_EASY: return 35;
        case SUDOKU_DIFFICULTY_MEDIUM: return 45;
        case SUDOKU_DIFFICULTY_HARD: return 55;
        default: return 45;
    }
}

static const char* difficulty_label(SudokuDifficulty difficulty) {
    switch (difficulty) {
        case SUDOKU_DIFFICULTY_EASY: return "Easy";
        case SUDOKU_DIFFICULTY_MEDIUM: return "Medium";
        case SUDOKU_DIFFICULTY_HARD: return "Hard";
        default: return "Medium";
    }
}

SudokuResult sudoku_generate_puzzle(
    SudokuBoard* out_puzzle,
    SudokuBoard* out_solution,
    SudokuDifficulty difficulty
) {
    if (!out_puzzle || !out_solution) return SUDOKU_ERR_INVALID_ARG;
    seed_if_needed();

    SudokuResult r = sudoku_generate_solution(out_solution);
    if (r != SUDOKU_OK) return r;

    sudoku_copy(out_puzzle, out_solution);

    const int target_holes = sudoku_holes_for_difficulty(difficulty);

    //attempt to remove numbers randomly; after each removal, check solvable
    //not the fastest method, but fine for our use case here (simple student project)
    int tries = 0;
    const int max_tries = 2000;
    while (count_holes(out_puzzle) < target_holes && tries < max_tries) {
        ++tries;

        int rr = rand_int(9);
        int cc = rand_int(9);
        if (out_puzzle->cell[rr][cc] == 0) continue;

        int saved = out_puzzle->cell[rr][cc];
        out_puzzle->cell[rr][cc] = 0;

        SudokuBoard tmp;
        sudoku_copy(&tmp, out_puzzle);
        if (sudoku_solve(&tmp) != SUDOKU_OK) {
            //revert removal if it makes it unsolvable
            out_puzzle->cell[rr][cc] = saved;
        }
    }

    //even if we didn't reach target holes (rare), it's still a valid solvable puzzle
    return SUDOKU_OK;
}

//html export


static void fprint_html_escaped(FILE* f, const char* s) {
    //very small escaper for titles
    for (const char* p = s ? s : ""; *p; ++p) {
        switch (*p) {
            case '&': fputs("&amp;", f); break;
            case '<': fputs("&lt;", f); break;
            case '>': fputs("&gt;", f); break;
            case '"': fputs("&quot;", f); break;
            case '\'': fputs("&#39;", f); break;
            default: fputc(*p, f); break;
        }
    }
}

static int is_css_safe_char(char ch) {
    //allow only a conservative subset for inline CSS values
    //if you pass "#dabfae" or "rgb(1,2,3)" it will work
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) return 1;
    if (ch >= '0' && ch <= '9') return 1;
    switch (ch) {
        case '#':
        case '(':
        case ')':
        case ',':
        case '.':
        case '%':
        case ' ':
        case '-':
        case '_':
            return 1;
        default:
            return 0;
    }
}

static void fprint_css_value(FILE* f, const char* s) {
    //print a css value, but drop unsafe characters to avoid breaking the page
    for (const char* p = s ? s : ""; *p; ++p) {
        if (is_css_safe_char(*p)) fputc(*p, f);
    }
}

SudokuResult sudoku_write_html_page(
    const char* html_path,
    const char* css_href,
    const SudokuBoard* puzzle,
    const SudokuTheme* theme,
    SudokuDifficulty difficulty
) {
    return sudoku_write_html_page_with_solution(
        html_path, css_href, puzzle, NULL, theme, difficulty
    );
}

static int board_is_filled_1_9(const SudokuBoard* b) {
    if (!b) return 0;
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            if (!is_in_range_1_9(b->cell[r][c])) return 0;
        }
    }
    return 1;
}

static void fprint_solution_attr(FILE* f, const SudokuBoard* solved) {
    //prints row-major 81 digits (1..9), caller must ensure solved is valid
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            fputc('0' + solved->cell[r][c], f);
        }
    }
}

SudokuResult sudoku_write_html_page_with_solution(
    const char* html_path,
    const char* css_href,
    const SudokuBoard* puzzle,
    const SudokuBoard* out_solution,
    const SudokuTheme* theme,
    SudokuDifficulty difficulty
) {
    if (!html_path || !css_href || !puzzle) return SUDOKU_ERR_INVALID_ARG;

    FILE* f = fopen(html_path, "w");
    if (!f) return SUDOKU_ERR_IO;

    const char* title = (theme && theme->page_title) ? theme->page_title : "Sudoku";

    //header
    fputs("<!DOCTYPE html>\n", f);
    fputs("<html lang=\"en\">\n", f);
    fputs("<head>\n", f);
    fputs("    <meta charset=\"utf-8\">\n", f);
    fputs("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n", f);
    fputs("    <title>", f);
    fprint_html_escaped(f, title);
    fputs("</title>\n", f);
    fputs("    <link rel=\"stylesheet\" href=\"", f);
    fprint_html_escaped(f, css_href);
    fputs("\">\n", f);
    fputs("    <script src=\"sudoku.js\" defer></script>\n", f);

    if (theme && (theme->panel_bg || theme->cell_hover_bg)) {
        fputs("    <style>\n", f);
        if (theme->panel_bg) {
            fputs("      header h1, main .game, main .difficulty, main .leaderboard { background-color: ", f);
            fprint_css_value(f, theme->panel_bg);
            fputs("; }\n", f);
        }
        if (theme->cell_hover_bg) {
            fputs("      main .game .container .cell:hover { background-color: ", f);
            fprint_css_value(f, theme->cell_hover_bg);
            fputs("; }\n", f);
        }
        //make given cells stand out a bit
        fputs("      .cell.given { display:flex; align-items:center; justify-content:center; font-weight:bold; font-size: 1.2em; }\n", f);
        fputs("      .cell.empty { display:flex; align-items:center; justify-content:center; color:#666; }\n", f);
        fputs("    </style>\n", f);
    } else {
        fputs("    <style>\n", f);
        fputs("      .cell { display:flex; align-items:center; justify-content:center; font-weight:bold; font-size: 1.2em; }\n", f);
        fputs("      .cell.empty { font-weight: normal; color:#666; }\n", f);
        fputs("    </style>\n", f);
    }

    fputs("</head>\n", f);
    fputs("<body>\n", f);
    fputs("    <header>\n", f);
    fputs("        <h1>", f);
    fprint_html_escaped(f, title);
    fputs("</h1>\n", f);
    fputs("        <br><br>\n", f);
    fputs("    </header>\n", f);

    fputs("    <main>\n", f);
    fputs("        <div class=\"game\">\n", f);
    fputs("            <div class=\"score\">\n", f);
    fputs("                <div class=\"time\">Time: 10:00</div>\n", f);
    fputs("                <div class=\"points\">Score: 0</div>\n", f);
    fputs("                <div class=\"mistakes\">Mistakes: 0/3</div>\n", f);
    fputs("            </div>\n", f);
    fputs("            <div class=\"container\"", f);
    if (out_solution && board_is_filled_1_9(out_solution)) {
        fputs(" data-solution=\"", f);
        fprint_solution_attr(f, out_solution);
        fputs("\"", f);
    }
    fputs(">\n", f);

    //81 cells: row-major
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            int v = puzzle->cell[r][c];
            if (v == 0) {
                fputs("                <div class=\"cell empty\"></div>\n", f);
            } else {
                fputs("                <div class=\"cell given\">", f);
                fputc('0' + v, f);
                fputs("</div>\n", f);
            }
        }
    }

    fputs("            </div>\n", f);
    fputs("        </div>\n", f);

    //diff panel
    fputs("        <div class=\"difficulty\">\n", f);
    fputs("            <h2>Difficulty</h2>\n", f);
    fputs("            <ul>\n", f);

    const char* lbl = difficulty_label(difficulty);
    fputs("                <li", f);
    if (strcmp(lbl, "Easy") == 0) fputs(" class=\"active\"", f);
    fputs("><a href=\"sudoku_easy.html\">Easy</a></li>\n", f);
    fputs("                <li", f);
    if (strcmp(lbl, "Medium") == 0) fputs(" class=\"active\"", f);
    fputs("><a href=\"sudoku_medium.html\">Medium</a></li>\n", f);
    fputs("                <li", f);
    if (strcmp(lbl, "Hard") == 0) fputs(" class=\"active\"", f);
    fputs("><a href=\"sudoku_hard.html\">Hard</a></li>\n", f);

    fputs("            </ul>\n", f);
    fputs("            <div class=\"buttons\">\n", f);
    fputs("                <button class=\"b\" data-action=\"start\">Start</button>\n", f);
    fputs("                <button class=\"b\" data-action=\"pause\">Pause</button>\n", f);
    fputs("                <button class=\"b\" data-action=\"reset\">Reset</button>\n", f);
    fputs("            </div>\n", f);
    fputs("        </div>\n", f);

    //leaderboard (currently left completely static, as implementing it would add a ton of complexity
    fputs("        <div class=\"leaderboard\">\n", f);
    fputs("            <h2>Leaderboard</h2>\n", f);
    fputs("            <ol>\n", f);
    fputs("                <li>Malunke</li>\n", f);
    fputs("                <li>Andrius</li>\n", f);
    fputs("                <li>Adomas</li>\n", f);
    fputs("                <li>Irmantas</li>\n", f);
    fputs("                <li>Arvydas</li>\n", f);
    fputs("                <li>Luna</li>\n", f);
    fputs("                <li>Gabija</li>\n", f);
    fputs("                <li>Augustas</li>\n", f);
    fputs("                <li>Kostas</li>\n", f);
    fputs("                <li>Justas</li>\n", f);
    fputs("            </ol>\n", f);
    fputs("        </div>\n", f);

    fputs("    </main>\n", f);
    fputs("    <footer></footer>\n", f);
    fputs("</body>\n", f);
    fputs("</html>\n", f);

    fclose(f);
    return SUDOKU_OK;
}

