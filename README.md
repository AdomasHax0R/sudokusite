# sudokusite
Project for the procedural programming course of Vilnius University.


# sudoku page generator (Stage 2 Module)

This repo contains a small c module that can:

- generate a sudoku solution
- generate a sudoku puzzle (some cells removed)
- export a static HTML page with the puzzle pre-filled
- optionally produce a tiny interactive page (via `sudoku.js`, no dependencies)

## files / architecture

- **`sudoku_module.h`**
  - Public API (types + function declarations)

- **`sudoku_module.c`**
  - Implementation of the module.
  - Contains:
    - backtracking solver
    - random solution generator
    - puzzle generation by “remove numbers and check solvable”
    - HTML export function that writes a page compatible with your layout/CSS

- **`example_generate_page.c`**
  - Minimal demo program that uses the module.
  - Generates a puzzle and writes `generated_sudoku.html`.

- **`style.css`, `background.png`, `MAGNETOB.TTF`**
  - The static styling/assets for the page.
- **`sudoku.js`**
  - Tiny browser script to make the generated page playable (click + keyboard + timer).

## data model

The board is stored as a simple 9×9 integer array:

- `0` means **empty cell**
- `1..9` are Sudoku values

In code this is:

- `SudokuBoard.cell[row][col]`
- `row` and `col` are in range `0..8`

## how the solver works (simple backtracking)

`sudoku_solve()` uses a simple algo:

1. Find the first empty cell (value `0`), scanning row major.
2. Try numbers `1..9` (shuffled for variety).
3. For each number, check if it can be placed:
   - not already in the same row
   - not already in the same column
   - not already in the same 3×3 box
4. Place it and recursively solve the next empty cell.
5. If it gets stuck, undo the placement (backtrack) and try the next number.

This is not the most efficient solver, but it’s easy to understand and good enough for a small project.

## how solution generation works

`sudoku_generate_solution()`:

- starts from an empty board
- runs the same backtracking solver
- because it shuffles the tried numbers, it produces different solutions (if you seed RNG)

randomness:

- call `sudoku_seed(time(NULL))` in your program if you want different output each run
- if you don’t call `sudoku_seed()`, the module behaves deterministically

## how puzzle generation works

`sudoku_generate_puzzle()` does:

1. Generate a full valid solution.
2. Copy solution -> puzzle.
3. Remove numbers at random positions until we reach the target “holes” count:
   - Easy: 35 empty cells
   - Medium: 45 empty cells
   - Hard: 55 empty cells
4. After removing a number, check if the puzzle is still solvable:
   - make a temporary copy
   - try to solve it
   - if it becomes unsolvable, revert that removal

important note:

- The puzzle is guaranteed to be solvable
- It is not guaranteed to have a unique solution (uniqueness checking would add complexity)

Cell classes:

- given value: `<div class="cell given">5</div>`
- empty: `<div class="cell empty"></div>`

This works with your existing `style.css` because the grid/`.cell` elements are the same.
The function also injects a tiny `<style>` block so the numbers are centered and readable.

### Optional browser interactivity (still simple)

If `sudoku.js` is present next to the generated HTML file, the page becomes playable:

- click an empty cell to select it
- type `1..9` to fill, `Backspace/Delete` to clear
- Start/Pause/Reset buttons run a simple timer

If you want the page to detect wrong inputs, generate HTML **with** the solution embedded:

- use `sudoku_write_html_page_with_solution(...)`
- the module will add `data-solution="..."` (81 digits) for `sudoku.js` to validate against

Optional theming:

- pass `SudokuTheme* theme` (or `NULL`)
- you can override:
  - panel background color
  - cell hover color
  - page title

## Building / running the demo

From the repo root:

```bash
gcc -std=c99 -O2 -Wall -Wextra -pedantic sudoku_module.c example_generate_page.c -o gen_page
./gen_page
```

Output:

- `generated_sudoku.html` (open it in a browser; it uses `style.css`)

Note for deployment/publishing:

- keep `generated_sudoku.html`, `style.css`, `sudoku.js`, `background.png`, `MAGNETOB.TTF` in the same folder

## Stage 3 console app (static “site” generator)

This repo also includes a simple Stage 3 console application (`sudoku_app.c`) that generates a small static website:

- `index.html`
- `sudoku_easy.html`
- `sudoku_medium.html`
- `sudoku_hard.html`

The difficulty buttons work by **navigating** between these pre-generated HTML files (still a static site; no backend).

Build:

```bash
gcc -std=c99 -O2 -Wall -Wextra -pedantic sudoku_module.c sudoku_app.c -o sudoku_app
```

Run:

```bash
./sudoku_app --all
```

Then open `index.html` in your browser (and publish the whole folder).


## Limitations (by design)

- Browser validation is optional: it only works if you embed a solution.
- Puzzle uniqueness is not enforced.
- Uses `rand()` (simple, good enough for a student project).


