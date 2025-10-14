# Hex (C++)

Small console Hex game with a Monte Carlo bot. Default board size is 11×11. You can play as Blue (top–bottom) or Red (left–right).

## Build

Requirements: C++17 compiler, CMake ≥3.15.

```bash
git clone https://github.com/DavidH2802/hex.git
cd hex
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run

```bash
./build/HexGame        # Windows: .\build\HexGame.exe
```

The program will prompt for your color and then for moves (row, column). The bot’s iteration count and board size are set in code; search for the iteration parameter and the board-size constant to adjust.

## Performance (author’s tests)

* 11×11 board, 2,000 iterations: max ~1.2 s per bot move
* 11×11 board, 5,000 iterations: max ~11 s per bot move

## Time Complexity

Let ( n ) be the board dimension (cells ( N = n^2 )) and ( I ) the number of Monte Carlo iterations per move.

* **Single playout:** ( O(N) )
* **Bot move (evaluating candidates via ( I ) playouts):** ( O(I \cdot N) = O(I \cdot n^2) )
* **Win check (DFS/union-find style):** ( O(N) ) per check
* **Space:** ( O(N) )

## Project Structure

* `src/` – game loop, board, bot, controller
* `include/` – headers
* `CMakeLists.txt` – build configuration
