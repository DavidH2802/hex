#pragma once

#include "types.h"
#include "board.h"
#include "bot.h"

using namespace std;

class GameController{
    private:
        Square_State player_colour;

    public:
        GameController ();

        bool blue_won(const Board& board);

        bool red_won(const Board& board);

        void print_board(const Board& board) const;

        void make_move(const Square_State& player_colour, Board& board);
};
