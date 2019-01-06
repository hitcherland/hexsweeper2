#include "game.h"

void setup( int radius ) {
    generate_layout( radius );
}

void place_mines( float mine_ratio, Cell *ignore_cell ) {
	int count = 3 * radius * ( radius + 1 ) + 1;
	int total_mine_count = mine_ratio * count;

    int ignore_h = ignore_cell->h;
    int ignore_j = ignore_cell->j;
    std::cout << ignore_h << ", " << ignore_j << std::endl;

    std::shuffle(std::begin(cells), std::end(cells), rng);
    for( std::vector<Cell>::iterator it = cells.begin(); it != cells.end(); ++it) {
        int index = it - cells.begin();
        Cell cell = *(it);
        reset_cell( cell );
        if( index < total_mine_count ) {
            int h = it->h;
            int j = it->j;
            std::cout << ignore_h << ", " << ignore_j << " --> " << h << ", " << j <<std::endl;
            if( ignore_h == h && ignore_j == j)
                continue;
            it->is_mine = true;

            Cell* n;
            n = find_cell( h, j + 1 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h, j - 1 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h + 1, j ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h - 1, j ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h + 1, j - 1 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h - 1, j + 1 ); if( n != NULL ) n->mine_count += 1;
            //next nearest 
            /*
            n = find_cell( h + 2, j - 1 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h - 2, j + 1 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h + 1, j - 2 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h - 1, j + 2 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h - 1, j - 1 ); if( n != NULL ) n->mine_count += 1;
            n = find_cell( h + 1, j + 1 ); if( n != NULL ) n->mine_count += 1;
            */
        }
    }
}

void reveal( Cell *cell ) {
    if( cell->revealed || cell->flagged)
        return;

    cell->revealed = true;
    display_reveal( cell );
}

void flag( Cell &cell ) {
    cell.flagged = ! cell.flagged; 
    display_flag( cell );
}

void generate_layout( int r ) {
    std::cout << "generating layout" << std::endl;
    display_clean( r );
    cells.clear();
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
