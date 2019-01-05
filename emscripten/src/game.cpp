#include "game.h"

void setup( int radius ) {
    generate_layout( radius );
}

void place_mines( float mine_ratio, Cell &cell ) {
	int count = 3 * radius * ( radius + 1 ) + 1;
	int total_mine_count = mine_ratio * count;

    std::shuffle(std::begin(cells), std::end(cells), rng);
    std::cout << radius << " vs. " << total_mine_count << std::endl;
    for( std::vector<Cell>::iterator it = cells.begin(); it != cells.end(); ++it) {
        int index = it - cells.begin();
        Cell cell = *(it);
        reset_cell( cell );
        if( index < total_mine_count ) {
            it->is_mine = true;
        }
    }
}

void reveal( Cell &cell ) {
    if( cell.revealed || cell.flagged)
        return;

    cell.revealed = true;
    display_reveal( cell );
}

void flag( Cell &cell ) {
    cell.flagged = ! cell.flagged; 
    display_flag( cell );
}

void generate_layout( int r ) {
    std::cout << "generating layout" << std::endl;
    display_clean( r );
    radius = r;
    for( int h=-radius; h<=radius; h++) {
        for( int j=-radius; j<=radius; j++) {
            if( std::abs( h + j ) > radius || std::abs( h ) > radius || std::abs( j ) > radius )
                continue;

            cells.emplace_back( h, j );
            Cell cell = cells.back();
            display_create_cell( cell );
        }
    }
}

Cell* find_cell( int h, int j ) {
    for( std::vector<Cell>::iterator it = cells.begin(); it != cells.end(); ++it)
        if( it->h == h && it->j == j )
            return &(*it);
    return NULL;
}
