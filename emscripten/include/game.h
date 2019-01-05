#ifndef __sweeper_game_h__
#define __sweeper_game_h__
    #include <cstdlib>
    #include <iostream>
    #include <sstream>
    #include <string>
    #include <algorithm>
    #include <random>
    #include <vector>

    enum ClickMode { SELECT, REVEAL, FLAG };
    static ClickMode click_mode = REVEAL;

    typedef struct Cell {
        int h;
        int j;
        int mine_count;
        bool is_mine;
        bool revealed;
        bool flagged;
        Cell( int h, int j ): 
            h(h), 
            j(j), 
            mine_count(0), 
            is_mine(false), 
            revealed(false),
            flagged( false )
        {}
    } Cell;

    static Cell* focused_cell;
    static std::vector<Cell> cells;

    static auto rng = std::default_random_engine {};
    static bool fullscreen = false;
    static double LONG_PRESS = 500;
    static double flag_timestamp, reveal_timestamp;
    static int radius = 4;
    static double mine_ratio = 0.3;

    void place_mines( float mine_ratio, Cell &cell );
    void reset_cell( Cell &cell );
    void generate_layout( int radius );
    void setup( int radius );

    void reveal( Cell &cell );
    void flag( Cell &cell );
    void display_clean( int radius );
    void display_flag( Cell &cell );
    void display_reveal( Cell &cell );
    void display_create_cell( Cell &cell );

    void cycle_click_mode();
    void set_click_mode( ClickMode mode );
    
    Cell* find_cell( int h, int j );

#endif 
