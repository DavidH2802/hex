#pragma once

#include "types.h"

#include <vector>
#include <utility>     
#include <tuple>    

using namespace std;

class Bot;

class Board { //Board implemented as Graph
    private:
        int v; // Number of vertices
        int x; // Board dimensions
        int y;
        vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>> adj; // Adjacency list representation

    public:
        Board();

        friend class GameController;

        friend class Bot;
};