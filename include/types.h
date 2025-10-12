#pragma once

enum class Square_State : short {FREE, BLUE, RED};

constexpr char to_char(Square_State s) {
    switch (s) {
        case Square_State::FREE: return '.';
        case Square_State::BLUE: return 'B';
        case Square_State::RED:  return 'R';
        default:                 return '?';
    }
}
