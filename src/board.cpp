#include "board.h"
#include "constants.h"

#include <vector>
#include <tuple>
#include <utility>
#include <stdexcept>

using namespace std;

Board::Board()
    : v(BOARD_SIZE*BOARD_SIZE),
      x(BOARD_SIZE),
      y(BOARD_SIZE),
      adj(BOARD_SIZE, vector<pair<Square_State, vector<tuple<int, int, double>>>>(BOARD_SIZE)) {
    if (v <= 0) {
        throw invalid_argument("Board size must be positive");
    }

    const vector<pair<int, int>> dirs = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, 1}, {1, -1}
    };

    for (int row = 0; row < y; ++row) { // initalizing edges of board represented as Graph
        for (int col = 0; col < x; ++col) {
            adj[row][col].first = Square_State::FREE;
            for (auto [dr, dc] : dirs) {
                const int nr = row + dr;
                const int nc = col + dc;
                if (nr >= 0 && nr < y && nc >= 0 && nc < x) {
                    adj[row][col].second.emplace_back(nr, nc, 1.0);
                }
            }
        }
    }
}
