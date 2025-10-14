#include "game_controller.h"

#include <stdexcept>

int main(){
    try{
        GameController game; // starting game
        return 0;
    }
    catch (runtime_error& e){
        return 1;
    }
}
