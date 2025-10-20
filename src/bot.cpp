#include "bot.h"
#include "constants.h"
#include "thread_pool.h"

#include <vector>
#include <tuple>         
#include <utility>       
#include <stdexcept>
#include <algorithm>
#include <array> 
#include <stack>
#include <random>
#include <cmath>
#include <thread>
#include <numeric>
#include <cassert>
#include <chrono>
#include <iostream>

using namespace std;

Bot::Bot (const Square_State& colour) : colour(colour), flat_board(BOARD_SIZE*BOARD_SIZE), neighbors(BOARD_SIZE*BOARD_SIZE), mask(BOARD_SIZE*BOARD_SIZE), pool(thread::hardware_concurrency()), num_threads(thread::hardware_concurrency()) {
    const array<pair<int,int>,6> dirs = {{ // neighbour map
        {-1, 0}, {-1, +1}, {0, -1}, {0, +1}, {+1, -1}, {+1, 0}
    }};
    for (int r = 0; r < BOARD_SIZE; ++r)
        for (int c = 0; c < BOARD_SIZE; ++c) {
            int i = r * BOARD_SIZE + c;
            for (auto [dr, dc] : dirs) {
                int nr = r + dr, nc = c + dc;
                if (0 <= nr && nr < BOARD_SIZE && 0 <= nc && nc < BOARD_SIZE)
                    neighbors[i].push_back(nr * BOARD_SIZE + nc);
            }
        }

    thread_boards.resize(num_threads, vector<Square_State>(BOARD_SIZE*BOARD_SIZE));
}

pair<int, int> Bot::make_move (const vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>>& adj){ // Evaluate moves based on MC
    // Flattened board for MC:
    // - Contiguous memory => better cache locality in playout loops.
    // - Faster randomization: we can shuffle a flat index vector and map to (r,c).
    // - Simpler, branch-light inner loops during rollouts.
    // Complexity note: a playout still costs O(N), but with fewer cache misses and
    // less iterator overhead compared to nested 2D structures.

    const auto start = chrono::system_clock::now();
    for (int i = 0; i < BOARD_SIZE; ++i){
        for (int j = 0; j < BOARD_SIZE; ++j){
            flat_board[j+(BOARD_SIZE*i)] = adj[i][j].first;
        }
    }
    for (auto thread_board : thread_boards){
        thread_board = flat_board;
    }

    vector<int> free_indices;
    // get free indices
    free_indices.reserve(flat_board.size());
    for (int i = 0; i < flat_board.size(); ++i){
        if (flat_board[i] == Square_State::FREE){
            free_indices.push_back(i);
        }
    }

    vector<double> moves_evals(free_indices.size(), 0);

    assert(moves_evals.size() == free_indices.size());
    
    // Dual-strategy selection:
    // 1) Run a shallow exhaustive/tactical probe to detect forced wins/blocks.
    // 2) If nothing decisive is found (or frontier is too large), switch to Monte Carlo.
    // Rationale: exhaustive search is exponential in depth—use it only when the branch
    // factor is small or a tactic is likely; MC scales as O(I * N) and gives a robust
    // estimate over many playouts. Thresholds (depth/branch limits, I) can be tuned.

    // get number of possible combinations with free indices (log against overflowing)
    double logC = lgamma(free_indices.size()) - lgamma(((free_indices.size()-1)/2) + 1) - lgamma(free_indices.size() - (free_indices.size()-1)/2);
    if (logC > log(MC_MAX_ITERATIONS)){
        for (auto move : free_indices){ // every move checked
            flat_board[move] = this->colour;
            auto it = find(free_indices.begin(), free_indices.end(), move);
            if (it != free_indices.end()) {
                iter_swap(it, free_indices.end() - 1); // swap with last element
                free_indices.pop_back(); // remove last element
            }

            assert(std::all_of(free_indices.begin(), free_indices.end(),
                   [&](int x){ return flat_board[x] == Square_State::FREE; }));

            auto free_copy = free_indices;

            vector<double> partial_sums(num_threads, 0);
            int iterations_per_thread = MC_MAX_ITERATIONS/num_threads;

            for (int t = 0; t < num_threads; ++t){
                pool.enqueue([&, move, t, free_copy] (mt19937& rng) mutable{
                    int local_sum = 0;
                    for (int i = 0; i < iterations_per_thread; ++i){
                        local_sum += get_random_mc_iteration(move, rng, thread_boards[t], free_copy);
                    }
                    partial_sums[t] = local_sum;
                });
            }

            pool.wait_all();

            moves_evals[move] = accumulate(partial_sums.begin(), partial_sums.end(), 0.0);

            // add move back to free_indices
            flat_board[move] = Square_State::FREE;
            free_indices.push_back(move); 
        }
    }

    // Exhaustive tactical check (shallow).
    // Purpose: catch immediate wins/blocks and short forcing sequences that MC rollouts
    // can miss due to variance. We intentionally cap depth/width because complexity
    // grows ~O(b^d), where b ≈ number of legal moves and d = lookahead depth.
    // If this block finds a forced win/block, we play it; otherwise we fall back to MC.
    // Trade-off: tiny extra upfront time, but big reduction in obvious tactical blunders.
    else{
        for (auto move: free_indices){
            flat_board[move] = this->colour;
            auto it = find(free_indices.begin(), free_indices.end(), move);
            if (it != free_indices.end()) {
                iter_swap(it, free_indices.end() - 1); // swap with last element
                free_indices.pop_back(); // remove last element
            }

            mask.resize(free_indices.size(), 0); // binary mask for giving combinations
            fill(mask.begin(), mask.begin() + (free_indices.size()/2), 1);

            int total_score = 0;
            do {
                // Fill board according to mask
                for (int i = 0; i < free_indices.size(); ++i){
                    if (mask[i] == 1){
                        flat_board[free_indices[i]] = this->colour;
                    }
                    else{
                        flat_board[free_indices[i]] = (this->colour == Square_State::BLUE ? Square_State::RED : Square_State::BLUE);
                    }
                }
                    

                // Evaluate and record outcome
                total_score += check_win(flat_board);

                // Reset board for next combination
                for (int idx : free_indices)
                    flat_board[idx] = Square_State::FREE;

            } while (std::prev_permutation(mask.begin(), mask.end())); // iterates over all unique k-combinations
            moves_evals[move] = total_score;
        }
    }
    // return best move
    int best_k = distance(moves_evals.begin(), max_element(moves_evals.begin(), moves_evals.end()));
    int best_move = free_indices[best_k];

    const auto end = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout<< "Bot thought for " << duration.count() << "ms\n";
    return {best_move/BOARD_SIZE, best_move%BOARD_SIZE};
}

int Bot::get_random_mc_iteration(int move, mt19937& rng, vector<Square_State>& board, vector<int>& shuffled){
    shuffle(shuffled.begin(), shuffled.end(), rng); // random mc

    int half = shuffled.size() / 2;
    for (int i = 0; i < shuffled.size(); ++i) { // fill half of free_indices with Blue, other with Red
        if (i < half){
            board[shuffled[i]] = this->colour;
        }
        else{
            if (this->colour == Square_State::BLUE){
                board[shuffled[i]] = Square_State::RED;
            }
            else{
                board[shuffled[i]] = Square_State::BLUE;
            }
        }
    }
    int win = check_win(board);

    for (int idx : shuffled){
        board[idx] = Square_State::FREE;
    }

    return win;
}

int Bot::check_win(const vector<Square_State>& board){
    vector<bool> visited(board.size(), false);
    stack<int> s;

    if (this->colour == Square_State::RED){
        for (int r = 0; r < BOARD_SIZE; ++r) { // starts at column 0 (looking trough rows) 
            int i = r * BOARD_SIZE;
            if (board[i] == Square_State::RED) {
                s.push(i);
                visited[i] = true;
            }
        }
        while (!s.empty()) {
            int i = s.top(); s.pop(); // getting last added neighbour
            int col = i % BOARD_SIZE;
            if (col == BOARD_SIZE - 1) return 1; // reached right edge: red win
            for (int nbr : neighbors[i]) { // add neighbour to stack
                if (!visited[nbr] && board[nbr] == Square_State::RED) {
                    visited[nbr] = true;
                    s.push(nbr);
                }
            }
        }

    } // same but for Blue and from top to bottom
    else if (this->colour == Square_State::BLUE){
        for (int c = 0; c < BOARD_SIZE; ++c){
            int i = c;
            if (board[i] == Square_State::BLUE){
                s.push(i);
                visited[i] = true;
            }
        }
        while(!s.empty()){
            int i = s.top(); s.pop();
            int row = i / BOARD_SIZE;
            if (row == BOARD_SIZE - 1) return 1; // reached bottom edge: blue win
            for (int nbr : neighbors[i]) {
                if (!visited[nbr] && board[nbr] == Square_State::BLUE) {
                    visited[nbr] = true;
                    s.push(nbr);
                }
            }
        }
    }
        
    return 0;
}
