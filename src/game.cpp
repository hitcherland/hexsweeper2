#include "game.h"

int* choose_in_range( int range, int count ) {
    int c=0;
    int *output = (int*) malloc( ( count + 1 ) * sizeof(int) );
    while( c < count && c < range ) {
        int r = rand() % range;
        int pass = 0;
        for( int i=0; i<c; i++) {
            if( output[ i ] == r ) {
                pass = 1;
                break;
            }
        }
        if( pass == 0 ) {
            output[c++] = r;
        }
    }
    output[ c ] = NULL;
    return output;
}
