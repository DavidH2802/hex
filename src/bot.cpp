#include "bot.h"
#include "constants.h"

#include <vector>
#include <tuple>         
#include <utility>       
#include <stdexcept>
#include <algorithm>
#include <array> 
#include <stack>
#include <random>
#include <cmath> 

using namespace std;

Bot::Bot (const Square_State& colour) : colour(colour), flat_board(BOARD_SIZE*BOARD_SIZE), neighbors(BOARD_SIZE*BOARD_SIZE), visited(BOARD_SIZE*BOARD_SIZE, false), mask(),rng(random_device{}()) {
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
}

pair<int, int> Bot::make_move (const vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>>& adj){ // Evaluate moves based on MC
    for (int i = 0; i < BOARD_SIZE; ++i){ // flatten current adj
        for (int j = 0; j < BOARD_SIZE; ++j){
            flat_board[j+(BOARD_SIZE*i)] = adj[i][j].first;
        }
    }
    vector<int> free_indices;
    // get free indices
    free_indices.reserve(flat_board.size());
    for (int i = 0; i < flat_board.size(); ++i){
        if (flat_board[i] == Square_State::FREE){
            free_indices.push_back(i);
        }
    }

    vector<int> moves_evals(flat_board.size(), 0);
    // get number of possible combinations with free indices (log against overflowing)
    double logC = exp(lgamma(free_indices.size()) - lgamma(((free_indices.size()-1)/2) + 1) - lgamma(free_indices.size() - (free_indices.size()-1)/2));
    if (logC > log(MC_MAX_ITERATIONS)){
        for (auto move : free_indices){ // every move checked
            flat_board[move] = this->colour;
            auto it = find(free_indices.begin(), free_indices.end(), move);
            if (it != free_indices.end()) {
                iter_swap(it, free_indices.end() - 1); // swap with last element
                free_indices.pop_back(); // remove last element
            }

            for (int i = 0; i < MC_MAX_ITERATIONS; ++i){
                moves_evals[move] += get_random_mc_iteration(free_indices, move); // add eval
            }
            // add move back to free_indices
            flat_board[move] = Square_State::FREE;
            free_indices.push_back(move); 
        }
    }
    else{ // all possible combinations (not so many as C <= MC_MAX_ITERATIONS)
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
                total_score += check_win();

                // Reset board for next combination
                for (int idx : free_indices)
                    flat_board[idx] = Square_State::FREE;

            } while (std::prev_permutation(mask.begin(), mask.end())); // iterates over all unique k-combinations
            moves_evals[move] = total_score;
        }
    }
    // return best move
    int idx = distance(moves_evals.begin(), max_element(moves_evals.begin(), moves_evals.end()));

    return {idx/BOARD_SIZE, idx%BOARD_SIZE};
}

int Bot::get_random_mc_iteration(vector<int>& free_indices, int move){
    shuffle(free_indices.begin(), free_indices.end(), rng); // random mc

    int half = free_indices.size() / 2;
    for (int i = 0; i < free_indices.size(); ++i) { // fill half of free_indices with Blue, other with Red
        if (i < half){
            flat_board[free_indices[i]] = this->colour;
        }
        else{
            if (this->colour == Square_State::BLUE){
                flat_board[free_indices[i]] = Square_State::RED;
            }
            else{
                flat_board[free_indices[i]] = Square_State::BLUE;
            }
        }
    }
    int win = check_win();

    for (int idx : free_indices){
        flat_board[idx] = Square_State::FREE;
    }

    return win;
}

int Bot::check_win(){
    // cleanup after previous run
    fill(visited.begin(), visited.end(), false);
    while(!s.empty()) s.pop();

    if (this->colour == Square_State::RED){
        for (int r = 0; r < BOARD_SIZE; ++r) { // starts at column 0 (looking trough rows) 
            int i = r * BOARD_SIZE;
            if (flat_board[i] == Square_State::RED) {
                s.push(i);
                visited[i] = true;
            }
        }
        while (!s.empty()) {
            int i = s.top(); s.pop(); // getting last added neighbour
            int col = i % BOARD_SIZE;
            if (col == BOARD_SIZE - 1) return 1; // reached right edge: red win
            for (int nbr : neighbors[i]) { // add neighbour to stack
                if (!visited[nbr] && flat_board[nbr] == Square_State::RED) {
                    visited[nbr] = true;
                    s.push(nbr);
                }
            }
        }

    } // same but for Blue and from top to bottom
    else if (this->colour == Square_State::BLUE){
        for (int c = 0; c < BOARD_SIZE; ++c){
            int i = c;
            if (flat_board[i] == Square_State::BLUE){
                s.push(i);
                visited[i] = true;
            }
        }
        while(!s.empty()){
            int i = s.top(); s.pop();
            int row = i / BOARD_SIZE;
            if (row == BOARD_SIZE - 1) return 1; // reached bottom edge: blue win
            for (int nbr : neighbors[i]) {
                if (!visited[nbr] && flat_board[nbr] == Square_State::BLUE) {
                    visited[nbr] = true;
                    s.push(nbr);
                }
            }
        }
    }
        
    return 0;
}
