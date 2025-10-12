#include "board.h"

#include <vector>
#include <tuple>
#include <utility>
#include <stdexcept>

using namespace std;

Board::Board(int width, int height)
    : v(width * height),
      x(width),
      y(height),
      adj(height, vector<pair<Square_State, vector<tuple<int, int, double>>>>(width)) {
    if (v <= 0) {
        throw invalid_argument("Number of vertices must be positive");
    }

    const vector<pair<int, int>> dirs = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, 1}, {1, -1}
    };

    for (int row = 0; row < y; ++row) {
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
