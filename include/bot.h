#pragma once

#include "types.h"

#include <vector>
#include <utility>
#include <tuple>

using namespace std;

class Bot{
    private:
        Square_State colour;
        vector<pair<int, int>> moves;

    public:
        Bot (const Square_State& colour);

        pair<int, int> make_move (const vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>>& adj);

        friend class GameController;
};