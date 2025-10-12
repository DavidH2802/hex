#include "bot.h"

#include <vector>
#include <tuple>         
#include <utility>       
#include <stdexcept>
#include <algorithm> 
#include <chrono>  

using namespace std;
using namespace chrono;


Bot::Bot (const Square_State& colour) : colour(colour), moves(0) {}

pair<int, int> Bot::make_move (const vector<vector<pair<Square_State, vector<tuple<int, int, double>>>>>& adj){
    const int height = static_cast<int>(adj.size());
    const int width = static_cast<int>(adj[0].size());

    if (this->colour == Square_State::BLUE){
        if (this->moves.empty()){
            int mid_col = width / 2;
            moves.push_back({0, mid_col});
            return {0, mid_col};
        }
        auto [row, col] = this->moves.back();

        vector<pair<int,int>> dirs = {
            {1, 0},   // down
            {1, -1},  // down-left
            {0, -1},  // left
            {0, 1},   // right
            {-1, 1},  // up-right
            {-1, 0}   // up  
        };

        for (auto [dr, dc] : dirs) {
            int nr = row + dr;
            int nc = col + dc;
            if (nr >= 0 && nr < height && nc >= 0 && nc < width && adj[nr][nc].first == Square_State::FREE) {
                moves.push_back({nr, nc});
                return {nr, nc};
            }
        }

        // fallback if surrounded
        for (int r = 0; r < height; ++r){
            for (int c = 0; c < width; ++c){
                if (adj[r][c].first == Square_State::FREE){
                    moves.push_back({r, c});
                    return {r, c};
                }
            }
        }
    }
    else if (this->colour == Square_State::RED){
        if (this->moves.empty()){
            int mid_row = height / 2;
            if (adj[mid_row][0].first == Square_State::FREE){
                moves.push_back({mid_row, 0});
                return {mid_row, 0};
            }
            else {
                int alt_row = std::min(mid_row + 1, height - 1);
                moves.push_back({alt_row, 0});
                return {alt_row, 0};
            }
        }
        auto [row, col] = this->moves.back();

        vector<pair<int,int>> dirs = {
            {0, 1},   // right
            {-1, -1}, // up-left
            {1, 0},   // down
            {-1, 0},  // up
            {1, -1},  // down-left
            {0, -1}   // left    
        };

        for (auto [dr, dc] : dirs) {
            int nr = row + dr;
            int nc = col + dc;
            if (nr >= 0 && nr < height && nc >= 0 && nc < width && adj[nr][nc].first == Square_State::FREE) {
                moves.push_back({nr, nc});
                return {nr, nc};
            }
        }

        // fallback if surrounded
        for (int r = 0; r < height; ++r){
            for (int c = 0; c < width; ++c){
                if (adj[r][c].first == Square_State::FREE){
                    moves.push_back({r, c});
                    return {r, c};
                }
            }
        }
    }
    else{
        throw runtime_error("Bot ecountered error");
    }
}