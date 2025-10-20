Hex (C++)

Small console Hex game with a dual-strategy bot: a light exhaustive tactical check for near-term forcing moves, and a Monte Carlo (MC) playout policy for evaluation. Default board is 11×11. You can play as Blue (top–bottom) or Red (left–right).

Build

Requirements: C++17 compiler, CMake ≥3.15.

git clone https://github.com/DavidH2802/hex.git
cd hex
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

Run
./build/HexGame        # Windows: .\build\HexGame.exe


The program prompts for your color and then move coordinates (row, col). Bot iteration count and board size are compile-time constants in the code.

What I learned

Dual strategy (exhaustive + Monte Carlo): Exhaustive checks are great at catching immediate wins/blocks and tiny tactical sequences; MC handles broader position evaluation without hand-tuned heuristics. The hybrid reduces blunders from pure randomness while avoiding the combinatorial blow-up of deep search.

Multithreading: The bot parallelizes Monte Carlo playouts across hardware threads using `std::thread::hardware_concurrency()`. A custom thread pool distributes iterations, significantly reducing computation time per move. Each thread maintains its own RNG for lock-free random number generation during playouts and also has the current flat_board in its own personal thread_boards buffer at its index.

Complexity vs. performance trade-offs:

Let n be board dimension and N = n². With I MC iterations, one bot decision costs ~O(I · N) because a single playout is O(N) and we repeat I times.

Exhaustive lookahead is exponential in depth (roughly O(b^d) with branching factor b ≈ #empty cells) and is therefore used only for very shallow/forced patterns.

Practical takeaway: keep exhaustive checks shallow, spend the rest of the budget on MC iterations.

Win checking optimization: Using DFS over the player's subgraph is O(N) per check. Dijkstra's algorithm is unnecessary because the graph is unweighted and we only need connectivity, not shortest paths.

Performance (author's tests, 4-core CPU)

11×11, 5,000 iterations: max ~4s per bot move

Time & Space Complexity

Single MC playout: O(N)

Bot move (MC): O(I · N)

Win check (DFS): O(N)

Space: O(N) for board + auxiliary structures

Project Structure

src/ – game loop, board, bot, controller, thread pool

include/ – headers

CMakeLists.txt – build configuration

Quick play example
Do you want to be blue or red (blue = 1, red = 2)? 1
Enter your move (row col): 5 5
Bot is thinking...
Bot plays: 6 5
...

Future improvements

Alpha-beta pruning / shallow minimax on top of the tactical layer (good for interviews; shows you can integrate classic search).

Smarter rollouts (e.g., bias playouts with connectivity heuristics or pattern moves rather than uniform random).

Dynamic thread pool sizing based on workload characteristics and available hardware resources.