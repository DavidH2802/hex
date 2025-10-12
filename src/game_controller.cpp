#include "game_controller.h"

#include <iostream>           
#include <vector>             
#include <stack>              
#include <utility>            
#include <tuple>              
#include <stdexcept>
#include <thread>             
#include <chrono>   

const int BOARD_SIZE = 11;

using namespace std;
using namespace std::chrono_literals;

GameController::GameController () : player_colour(Square_State::BLUE) { // default colour
    Bot bot(Square_State::BLUE);
    Board board = Board(BOARD_SIZE, BOARD_SIZE);
    
    int colour_player_tmp;
    int colour_bot;

    while(true){
        cout << "Do you want to be blue or red (blue = 1, red = 2)? ";
        cin >> colour_player_tmp;
        if ((colour_player_tmp != 1 && colour_player_tmp != 2)){
            cout << "Wrong input (only 0 or 1)\n";
        }
        else if (colour_player_tmp == 1){
            bot = Bot(Square_State::RED);
            player_colour = Square_State::BLUE;
            break;
        }
        else{
            player_colour = Square_State::RED;
            break;
        }
    }

    while (!blue_won(board) && !red_won(board)){
        if (player_colour == Square_State::BLUE){
            make_move(player_colour, board);
            print_board(board);
            if (blue_won(board)) break;
            cout << "Bot thinking...\n";
            this_thread::sleep_for(2s);
            pair <int, int> bot_move = bot.make_move(board.adj);
            board.adj[bot_move.first][bot_move.second].first = bot.colour;
            print_board(board);
        }
        else{
            cout << "Bot thinking...\n";
            this_thread::sleep_for(2s);
            pair<int, int> bot_move = bot.make_move(board.adj);
            board.adj[bot_move.first][bot_move.second].first = bot.colour;
            print_board(board);
            if(blue_won(board)) break;
            make_move(player_colour, board);
            print_board(board);
        }
    }
    if (blue_won(board)){
        cout<< "Blue player won!\n";
    }
    else if (red_won(board)){
        cout<< "Red player won!\n";
    }
    else{
        throw runtime_error("Game engine broke with no winner");
    }
    cout<<endl;
}

bool GameController::blue_won(const Board& board) {
    vector<vector<bool>> visited(board.y, vector<bool>(board.x, false));
    stack<pair<int, int>> s;

    for (int col = 0; col < board.x; ++col) {
        if (board.adj[0][col].first == Square_State::BLUE) {
            s.push({0, col});
            visited[0][col] = true;
        }
    }

    while (!s.empty()) {
        auto [row, col] = s.top();
        s.pop();

        if (row == board.y - 1) return true;

        for (const auto& neighbour : board.adj[row][col].second) {
            const auto& [nbrRow, nbrCol, weight] = neighbour;
            (void)weight;
            if (!visited[nbrRow][nbrCol] && board.adj[nbrRow][nbrCol].first == Square_State::BLUE) {
                visited[nbrRow][nbrCol] = true;
                s.push({nbrRow, nbrCol});
            }
        }
    }
    return false;
}

bool GameController::red_won(const Board& board) {
    vector<vector<bool>> visited(board.y, vector<bool>(board.x, false));
    stack<pair<int, int>> s;

    for (int row = 0; row < board.y; ++row) {
        if (board.adj[row][0].first == Square_State::RED) {
            s.push({row, 0});
            visited[row][0] = true;
        }
    }

    while (!s.empty()) {
        auto [row, col] = s.top();
        s.pop();

        if (col == board.x - 1) return true;

        for (const auto& neighbour : board.adj[row][col].second) {
            const auto& [nbrRow, nbrCol, weight] = neighbour;
            (void)weight;
            if (!visited[nbrRow][nbrCol] && board.adj[nbrRow][nbrCol].first == Square_State::RED) {
                visited[nbrRow][nbrCol] = true;
                s.push({nbrRow, nbrCol});
            }
        }
    }
    return false;
}

void GameController::print_board(const Board& board) const {
    for (int row = 0; row < board.y; ++row) {
        for (int indent = 0; indent < row; ++indent) cout << " ";
        for (int col = 0; col < board.x; ++col) {
            cout << to_char(board.adj[row][col].first);
            if (col < board.x - 1) cout << " - ";
        }
        cout << "\n";

        if (row < board.y - 1) {
            for (int indent = 0; indent <= row; ++indent) cout << " ";
            for (int col = 0; col < board.x - 1; ++col) {
                cout << "\\ / ";
            }
            cout << "\\\n";
        }
    }
}

void GameController::make_move(const Square_State& player_colour, Board& board){
    int row;
    int column;

    while (true) {
        if (player_colour == Square_State::BLUE) {
            cout << "You play North/South Side ";
        } else if (player_colour == Square_State::RED) {
            cout << "You play East/West Side ";
        } else {
            throw runtime_error("Game engine broke with no winner");
        }
        cout << "Row-Coordinate of your chosen square: ";
        cin >> row;
        cout << "Column-Coordinate of your chosen square: ";
        cin >> column;

        if (row >= board.y || column >= board.x || row < 0 || column < 0) {
            cout << "Square doesn't exist. Dimensions: " << board.x << "x" << board.y << "\n";
            cout << "Remember counting starts at 0!\n";
        } else if (board.adj[row][column].first != Square_State::FREE) {
            cout << "Square already taken, choose a different square!\n";
        } else {
            cout << "\n";
            board.adj[row][column].first = player_colour;
            break;
        }
    }
}