#ifndef __sweeper_game__
#define __sweeper_game__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "cell.h"

int* choose_in_range( int a, int b );

class GameManager {
    public:
        virtual void update_style();
};

class Game {
	public:
        int seed;
        int radius;
        int* mines;
        float mineRatio;
        int mineCount;
        Cell** cells;
        int cellCount;

        //virtual void init();
        //virtual void draw();
        //virtual void focusCell( Cell* cell );
        //virtual void unfocusCell( Cell* cell );
        //virtual void activateCell( Cell* cell );
};

#endif
