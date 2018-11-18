#ifndef __sweeper_html__
#define __sweeper_html__

#include <emscripten.h>
#include <emscripten/html5.h>

#include "game.h"
#include "html/cell.h"

class HTMLGame: public Game {
    public:
        HTMLGame();
        float cellRadius;

        void init();
        void show_states();
        HTMLCell* cells;
};

int main();

#endif
