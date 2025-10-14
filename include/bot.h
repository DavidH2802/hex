#pragma once

#include "types.h"

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
        vector<bool> visited;
        vector<int> mask;
        stack<int> s;
        mt19937 rng;

        int get_random_mc_iteration(vector<int>& free_indices, int move);
        int check_win();

    public:
        Bot (const Square_State& colour);

        pair<int, int> make_move (const vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>>& adj);

        friend class GameController;
};