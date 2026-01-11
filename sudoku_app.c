// sudoku_app.c - Stage 3 console application

// goal: generate a static sudoku website that can be published anywhere
//it produces:
//index.html (links to difficulties)
//sudoku_easy.html / sudoku_medium.html / sudoku_hard.html
//the pages are playable in the browser via sudoku.js (no external dependencies)

// Build:
//   gcc -std=c99 -O2 -Wall -Wextra -pedantic sudoku_module.c sudoku_app.c -o sudoku_app

// Run (interactive):
//   ./sudoku_app

// Run (non-interactive, generates all pages into current folder):
//   ./sudoku_app --all

#include "sudoku_module.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void read_line(char* buf, size_t n) {
    if (!buf || n == 0) return;
    if (!fgets(buf, (int)n, stdin)) {
        buf[0] = '\0';
        return;
    }
    // strip trailing newline
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
        buf[len - 1] = '\0';
        --len;
    }
}

static SudokuDifficulty parse_difficulty(const char* s, SudokuDifficulty fallback) {
    if (!s || !*s) return fallback;

    //simple, portable case insensitive compare (naudojam ascii).
    char tmp[32];
    size_t n = strlen(s);
    if (n >= sizeof(tmp)) n = sizeof(tmp) - 1;
    for (size_t i = 0; i < n; ++i) tmp[i] = (char)tolower((unsigned char)s[i]);
    tmp[n] = '\0';

    if (strcmp(tmp, "1") == 0 || strcmp(tmp, "easy") == 0) return SUDOKU_DIFFICULTY_EASY;
    if (strcmp(tmp, "2") == 0 || strcmp(tmp, "medium") == 0) return SUDOKU_DIFFICULTY_MEDIUM;
    if (strcmp(tmp, "3") == 0 || strcmp(tmp, "hard") == 0) return SUDOKU_DIFFICULTY_HARD;
    return fallback;
}

static const char* difficulty_file(SudokuDifficulty d) {
    switch (d) {
        case SUDOKU_DIFFICULTY_EASY: return "sudoku_easy.html";
        case SUDOKU_DIFFICULTY_MEDIUM: return "sudoku_medium.html";
        case SUDOKU_DIFFICULTY_HARD: return "sudoku_hard.html";
        default: return "sudoku_medium.html";
    }
}

static const char* difficulty_title_suffix(SudokuDifficulty d) {
    switch (d) {
        case SUDOKU_DIFFICULTY_EASY: return "Easy";
        case SUDOKU_DIFFICULTY_MEDIUM: return "Medium";
        case SUDOKU_DIFFICULTY_HARD: return "Hard";
        default: return "Medium";
    }
}

static int write_index_html(const char* css_href, const char* title, SudokuDifficulty active) {
    FILE* f = fopen("index.html", "w");
    if (!f) return 0;
    fputs("<!DOCTYPE html>\n", f);
    fputs("<html lang=\"en\">\n<head>\n", f);
    fputs("  <meta charset=\"utf-8\">\n", f);
    fputs("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n", f);
    fputs("  <title>", f);
    fputs(title ? title : "Sudoku", f);
    fputs("</title>\n", f);
    fputs("  <link rel=\"stylesheet\" href=\"", f);
    fputs(css_href ? css_href : "style.css", f);
    fputs("\">\n", f);
    fputs("</head>\n<body>\n", f);
    fputs("  <header><h1>", f);
    fputs(title ? title : "Sudoku", f);
    fputs("</h1><br><br></header>\n", f);
    fputs("  <main>\n", f);
    fputs("    <div class=\"difficulty\" style=\"width: 360px; height: auto;\">\n", f);
    fputs("      <h2>Choose difficulty</h2>\n", f);
    fputs("      <ul>\n", f);
    fputs("        <li", f);
    if (active == SUDOKU_DIFFICULTY_EASY) fputs(" class=\"active\"", f);
    fputs("><a href=\"sudoku_easy.html\">Easy</a></li>\n", f);
    fputs("        <li", f);
    if (active == SUDOKU_DIFFICULTY_MEDIUM) fputs(" class=\"active\"", f);
    fputs("><a href=\"sudoku_medium.html\">Medium</a></li>\n", f);
    fputs("        <li", f);
    if (active == SUDOKU_DIFFICULTY_HARD) fputs(" class=\"active\"", f);
    fputs("><a href=\"sudoku_hard.html\">Hard</a></li>\n", f);
    fputs("      </ul>\n", f);
    fputs("      <p style=\"font-family: sans-serif; font-weight: 600;\">\n", f);
    fputs("        Tip: the page is static. Difficulty switches by loading a different HTML file.\n", f);
    fputs("      </p>\n", f);
    fputs("    </div>\n", f);
    fputs("  </main>\n", f);
    fputs("  <footer></footer>\n", f);
    fputs("</body>\n</html>\n", f);
    fclose(f);
    return 1;
}

static int generate_one(SudokuDifficulty d, const char* css_href, const char* base_title, const SudokuTheme* base_theme) {
    SudokuBoard puzzle;
    SudokuBoard solution;

    SudokuResult r = sudoku_generate_puzzle(&puzzle, &solution, d);
    if (r != SUDOKU_OK) return 0;

    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "%s (%s)", base_title ? base_title : "Sudoku", difficulty_title_suffix(d));

    SudokuTheme theme = {0};
    if (base_theme) theme = *base_theme;
    theme.page_title = title_buf;

    r = sudoku_write_html_page_with_solution(
        difficulty_file(d),
        css_href ? css_href : "style.css",
        &puzzle,
        &solution,
        &theme,
        d
    );
    return r == SUDOKU_OK;
}

int main(int argc, char** argv) {
    sudoku_seed((unsigned int)time(NULL));

    const char* css_href = "style.css";
    const char* base_title = "Sudoku";

    SudokuTheme theme;
    theme.panel_bg = "#dabfae";
    theme.cell_hover_bg = "wheat";
    theme.page_title = base_title;

    int generate_all = 0;
    if (argc >= 2 && strcmp(argv[1], "--all") == 0) generate_all = 1;

    if (generate_all) {
        if (!write_index_html(css_href, base_title, SUDOKU_DIFFICULTY_MEDIUM)) {
            fprintf(stderr, "Failed to write index.html\n");
            return 1;
        }
        if (!generate_one(SUDOKU_DIFFICULTY_EASY, css_href, base_title, &theme) ||
            !generate_one(SUDOKU_DIFFICULTY_MEDIUM, css_href, base_title, &theme) ||
            !generate_one(SUDOKU_DIFFICULTY_HARD, css_href, base_title, &theme)) {
            fprintf(stderr, "Failed to generate one of the pages\n");
            return 1;
        }
        printf("OK: wrote index.html + sudoku_easy/medium/hard.html\n");
        return 0;
    }

    char diff_buf[32];
    char title_buf[128];

    printf("Sudoku generator (Stage 3)\n");
    printf("Difficulty (1=Easy, 2=Medium, 3=Hard) [2]: ");
    read_line(diff_buf, sizeof(diff_buf));
    SudokuDifficulty d = parse_difficulty(diff_buf, SUDOKU_DIFFICULTY_MEDIUM);

    printf("Page title [Sudoku]: ");
    read_line(title_buf, sizeof(title_buf));
    if (title_buf[0] == '\0') {
        base_title = "Sudoku";
    } else {
        base_title = title_buf;
    }

    theme.page_title = base_title;

    // Always (re)write the mini site so difficulty links work.
    if (!write_index_html(css_href, base_title, d)) {
        fprintf(stderr, "Failed to write index.html\n");
        return 1;
    }

    if (!generate_one(SUDOKU_DIFFICULTY_EASY, css_href, base_title, &theme) ||
        !generate_one(SUDOKU_DIFFICULTY_MEDIUM, css_href, base_title, &theme) ||
        !generate_one(SUDOKU_DIFFICULTY_HARD, css_href, base_title, &theme)) {
        fprintf(stderr, "Failed to generate sudoku pages\n");
        return 1;
    }

    printf("OK: wrote index.html + sudoku_easy/medium/hard.html\n");
    printf("Open index.html in your browser.\n");
    return 0;
}

