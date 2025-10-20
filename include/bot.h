#pragma once

#include "types.h"
#include "thread_pool.h"

#include <vector>
#include <utility>
#include <tuple>
#include <stack>
#include <random>

using namespace std;

class Bot{
    private:
        Square_State colour;
        // initialized once for faster mc iterations
        vector<Square_State> flat_board;
        vector<vector<int>> neighbors; 
        vector<int> mask;
        ThreadPool pool;
        vector<vector<Square_State>> thread_boards;
        int num_threads;

        int get_random_mc_iteration(int move, mt19937& rng, vector<Square_State>& board, vector<int>& shuffled);
        int check_win(const vector<Square_State>& board);

    public:
        Bot (const Square_State& colour);

        pair<int, int> make_move (const vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>>& adj);

        friend class GameController;
};