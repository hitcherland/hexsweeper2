#include <stdio.h>
#include <stdlib.h>
#include <new>

#include "html/main.h"

HTMLGame* game;

EM_JS(void, update_style, (
    float transition_speed,
    const char* stroke_,
    float unknown_stroke_width,
    const char* unknown_stroke_fill_,
    float hover_stroke_width,
    const char* free_fill_,
    const char* mine_fill_,
    const char* flag_fill_
    ), {
    var stroke = UTF8ToString( stroke_ );
    var unknown_stroke_fill = UTF8ToString( unknown_stroke_fill_ );
    var free_fill = UTF8ToString( free_fill_ );
    var mine_fill = UTF8ToString( mine_fill_ );
    var flag_fill = UTF8ToString( flag_fill_ );
    document.getElementById( "style" ).innerHTML = `
    * {
        transition: all ${transition_speed}s;
        font-family: mono;
        -webkit-user-select: none;  /* Chrome all / Safari all */
        -moz-user-select: none;     /* Firefox all */
        -ms-user-select: none;      /* IE 10+ */
        user-select: none;          /* Likely future */ 
        -webkit-touch-callout: none;
        -khtml-user-select: none;
        -webkit-tap-highlight-color: transparent;
    }

    .hex.focus > polygon {
        stroke: #00f;
    }    

    .hex.neighbour > polygon {
        stroke-width: ${hover_stroke_width};
    }    


    .hex { cursor: pointer; }
    .hex > polygon {
        stroke: ${stroke};
        stroke-width: ${unknown_stroke_width};
        fill: ${unknown_stroke_fill};
    }

    .hex > text {
        stroke: none;
        fill: ${stroke};
        font-weight: bold;
        opacity: 0;
    }

    .hex:hover > polygon {
        stroke-width: ${hover_stroke_width};
        z-index: 1;
    }

    .hex:hover > text {
        opacity: 1;
    }

    .hex.flag > polygon, .hex.mine > polygon, .hex.free > polygon {
        transition: all 0.5s;
    }
    .hex.flag > text, .hex.mine > text, .hex.free > text {
        opacity: 1;
    }

    .hex.free > polygon { fill: #008900; }
    .hex.mine > polygon { fill: #ec0000; }
    .hex.flag > polygon { fill: #b95c00; }
`;
});


EM_JS(void, make_svg, (), {
  var svg = document.createElementNS( "http://www.w3.org/2000/svg", "svg" );
  document.body.appendChild( svg );
  svg.setAttribute( "xmlns", "http://www.w3.org/2000/svg" );
  svg.setAttribute( "viewBox", "0 0 100 100" );
  svg.setAttribute( "id", "s" );
  svg.setAttribute( "version", "1.1" );

  var style = document.createElementNS( "http://www.w3.org/2000/svg", "style" );
  svg.appendChild( style );
  style.setAttribute( "id", "style" );
});

EM_JS(int, get_outer_radius, (), {
    var url = window.location.href;
    var name = "radius";
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)");
    var results = regex.exec(url);
    if (!results) return -1;
    if (!results[2]) return -2;
    return decodeURIComponent( results[2]);
});

HTMLGame::HTMLGame() {
    mineRatio = 0.1;
    seed = time( NULL );

    radius = get_outer_radius();
    if( radius < 0 ) radius = 3;

    cells = new HTMLCell[ cellCount ];
    cellCount = 1 + 3 * radius * ( radius + 1 );
    cellRadius = 57.1593533487 / ( 2 * radius + 1 );
    mineCount = round( cellCount * mineRatio );

    make_svg();
    update_style( 
        1.0,  // transition speed
        "#fff", // stroke style
        0.1 * cellRadius, // unknown cell stroke width
        "rgba(0,0,0,0)", // unknown cell fill
        0.2 * cellRadius, // hover stroke width
        "#008900", // free fill
        "#ec0000", // mine fill
        "#b95c00" // flag fill
    );


    float x, y;
    int rows[ 2 * radius + 1 ];
    float cartesianConverters[] = {
        1.7320 * cellRadius,
        0.8660 * cellRadius,
        1.5 * cellRadius
    };

    int index = 0;
    for( int i = -radius; i<= radius; i++ ) {
        for( int j = -radius; j <= radius; j++) {
            if( abs( i + j ) > radius )
                continue;
            x = 50 + j * cartesianConverters[ 0 ] + i * cartesianConverters[ 1 ];
            y = 50 + i * cartesianConverters[ 2 ];
            cells[ index ].init(  index, x, y, cellRadius );
            index++;
        }
        rows[ i + radius ] = index;
    }

    int row = 0;
    int prev_row_length = -1;
    int row_length = rows[ 0 ];
    int next_row_length = rows[ 1 ] - rows[ 0 ];

    for( int i=0; i<cellCount; i++) {
        if( i == rows[ row ] ) {
            row++;
            prev_row_length = row_length;
            row_length = rows[ row ] - rows[ row - 1 ];
            next_row_length = row < 2 * radius + 1 ? rows[ row + 1 ] - rows[ row ] : -1;
        }

        int top = rows[ row ] - row_length == i && row >= radius;
        int top_right = row == 2 * radius;
        int bottom_right = rows[ row ] == i + 1 && row >= radius;
        int bottom = rows[ row ] == i + 1 && row <= radius;
        int bottom_left = row == 0;
        int top_left = rows[ row ] - row_length == i && row <= radius;

		int neighbour_ids[ 6 ] = {
		    i + fmax( row_length, next_row_length ) - 1,
		    i + fmax( row_length, next_row_length ),
		    i + 1,
		    i - fmax( row_length, prev_row_length ) + 1,
		    i - fmax( row_length, prev_row_length ),
		    i - 1
		};

		HTMLCell** neighbours = cells[ i ].neighbours;
        neighbours[ 0 ] = !( top || top_right ) ? &cells[ int( i + fmax( row_length, next_row_length ) - 1  ) ]: NULL;
        neighbours[ 1 ] = !( top_right || bottom_right ) ? &cells[ int( i + fmax( row_length, next_row_length )  ) ]: NULL;
        neighbours[ 2 ] = !( bottom_right || bottom ) ? &cells[ int( i + 1  ) ]: NULL;
        neighbours[ 3 ] = !( bottom || bottom_left ) ? &cells[ int( i - fmax( row_length, prev_row_length ) + 1 ) ]: NULL;
        neighbours[ 4 ] = !( bottom_left || top_left ) ? &cells[ int( i - fmax( row_length, prev_row_length ) ) ]: NULL;
        neighbours[ 5 ] = !( top || top_left ) ? &cells[ int( i - 1 ) ]: NULL;
    }
    show_states();
}

void HTMLGame::init() {
    //read_json( "/styles/dark.json" );
    srand( seed );
    free( mines );
    mines = choose_in_range( cellCount, mineCount );

    for( int i=0; i<cellCount; i++) {
        cells[ i ].isMine = 0;
        cells[ i ].mineCount = 0;
    }

    for( int i=0; i<mineCount; i++) {
        HTMLCell* cell = &cells[ mines[ i ] ];
        cell->isMine = 1;

        for( int j=0; j<6; j++) {
            HTMLCell* neighbour = cell->neighbours[ j ];
            if( neighbour != NULL ) {
                neighbour->mineCount++;
            }
        }
    }
}

void HTMLGame::show_states() {
    for( int i=0; i<cellCount; i++) {
        HTMLCell* cell = &cells[ i ];
        printf("cell[%d]: pointer=%p id=%d, isMine=%d, mineCount=%d, activated=%d, neighbours@", i, cell,cell->id , cell->isMine, cell->mineCount, cell->activated);
        for( int j=0; j<6; j++) {
            HTMLCell* neighbour = cell->neighbours[ j ];
            printf( " %p", neighbour );
        }
        printf( "\n" );
    }
}

EM_BOOL click( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    game->show_states();

    return EM_TRUE;
}


int main() {
    game = new HTMLGame();
    //game->init();
    game->show_states();
    emscripten_set_mouseup_callback( "s", ( void* ) game, EM_TRUE, click ); 

    return 0;
}
