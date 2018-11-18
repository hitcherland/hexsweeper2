#ifndef __sweeper_game_h__
#define __sweeper_game_h__

#include <stdio.h>
#include <stdlib.h>

typedef struct Cell {
    int index;
    int is_mine;
    int neighbours[ 6 ];
    int neighbourMines;
    int position[ 2 ];
} Cell;

class Layout {
    public:
        Layout();
        int cellCount;
        Cell* cells;
        void setup_cell( Cell* cell );
        void setup_neighbours();
        void setup_neighbour( Cell* cell );
};

class ClassicHexagon: public Layout {
    public:
        void setup_cells( int r );
        void setup_neighbours();
        void setup_neighbour( Cell* cell );

    protected:
        int radius;
        int* rows;
        int top, top_right, bottom_right, bottom, bottom_left, top_left;
};

class Display {
    void init_cell( Cell* cell );
    void draw_cell( Cell* cell );
};

class Game {
    public:
        Game( Layout l, int s, int S, float m );
        Layout layout;
        int seed;
        int size;
        int* mines;
        float mineRatio;
};

#endif
