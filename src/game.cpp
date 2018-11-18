#include <math.h>
#include <stdlib.h>
#include "game.h"

//
//  LAYOUT SECTION
//

Layout::Layout() {
    cells = (Cell*) malloc( sizeof( Cell ) );
}

void Layout::setup_cell( Cell* cell ) {
    cell->is_mine = 0;
    cell->neighbourMines = 0;
};

//
// Classic Hexagon Layout
//

void ClassicHexagon::setup_cells( int r ) {
    radius = r;
    int count = 2 * radius + 1;

    free( rows );
    rows = (int*) malloc( count * sizeof( int ) );

    free( cells );
    cells = (Cell*) malloc( count * sizeof( Cell ) );

    cellCount = 0;
    for( int i = -radius; i<= radius; i++ ) {
        for( int j = -radius; j <= radius; j++) {
            if( abs( i + j ) > radius )
                continue;
            Cell cell = cells[ cellCount ];
            cell.index = cellCount;
            cell.is_mine = 0;

            for( int k = 0; k<6; k++ )
                cell.neighbours[ k ] = -1;

            cell.neighbourMines = 0;
            cell.position[ 0 ] = i;
            cell.position[ 1 ] = j;

            setup_cell( &cell );
            printf( "#0[ %d ]@%p|%p: %d,%d\n", cellCount, (void*) &cell, (void*) cells, cell.position[ 0 ], cell.position[ 1 ] );
            cellCount++;
        }
        rows[ i + radius ] = cellCount;
    }

    for( int i=0; i<cellCount; i++) {
        Cell cell = cells[ i ];
        printf( "#1[ %d ]@%p: %d,%d\n", i, (void*) &cell, cell.position[ 0 ], cell.position[ 1 ] );
    }
};

void ClassicHexagon::setup_neighbours() {
    printf( "setup neighbours: %d\n", cellCount );
    for(int i=0; i<cellCount; i++) { 
        Cell cell = cells[ i ];
        printf( "#2[ %d|%d ]: %d,%d\n", i,cells[i].index, cell.position[ 0 ], cell.position[ 1 ] );
        setup_neighbour( &cells[ i ]  ); 
    }
};

void ClassicHexagon::setup_neighbour( Cell* cell ) {
    int row = 0;
    int prev_row_length, row_length, next_row_length;

    while( cell->index >= rows[ row ] )
        row++;

    if( row == 0 )
        row_length = rows[ row ] - rows[ row - 1 ];
    else
        row_length = rows[ 0 ];

    if( row > 1 ) 
        prev_row_length = rows[ rows[ row - 1 ] - rows[ row - 2 ] ];
    else if( row == 1 )
        prev_row_length = rows[ 0 ];
    else
        prev_row_length = -1;

    if( row < cellCount )
        next_row_length = rows[ row + 1 ] - rows[ row ];
    else
        next_row_length = -1;

    int top = rows[ row ] - row_length == cell->index && row >= radius;
    int top_right = row == 2 * radius;
    int bottom_right = rows[ row ] == cell->index + 1 && row >= radius;
    int bottom = rows[ row ] == cell->index + 1 && row <= radius;
    int bottom_left = row == 0;
    int top_left = rows[ row ] - row_length == cell->index && row <= radius;


    cell->neighbours[ 0 ] = !( top || top_right ) ? cell->index + fmax( row_length, next_row_length ) - 1 : -1;
    cell->neighbours[ 1 ] = !( top_right || bottom_right ) ? cell->index + fmax( row_length, next_row_length ) : -1;
    cell->neighbours[ 2 ] = !( bottom_right || bottom ) ? cell->index + 1 : -1;
    cell->neighbours[ 3 ] = !( bottom || bottom_left ) ? cell->index - fmax( row_length, prev_row_length ) + 1: -1;
    cell->neighbours[ 4 ] = !( bottom_left || top_left ) ? cell->index - fmax( row_length, prev_row_length ): -1;
    cell->neighbours[ 5 ] = !( top || top_left ) ? cell->index - 1: -1;
};

Game::Game( Layout l, int s, int S, float m ) {
    layout = l;
    seed = s;
    size = S;
    mineRatio = m;
}
