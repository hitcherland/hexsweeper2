#include <time.h>
#include "game.h"
#include "html_display.h"

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

EM_JS(void, make_reload, (), {
  var reload = document.createElementNS( "http://www.w3.org/2000/svg", "path" );
  reload.setAttribute( "d", "M 12.007 8.01 L 12.007 8.488 L 12 8.488 C 12 10.974 9.985 12.988 7.5 12.988 C 5.015 12.988 3 10.974 3 8.488 C 3 6.17 4.759 4.284 7.012 4.037 L 7.012 6.549 L 11.519 3.775 L 7.012 1 L 7.012 3.013 C 4.203 3.26 2 5.615 2 8.488 C 2 11.526 4.462 13.988 7.5 13.988 C 10.509 13.988 12.95 11.571 12.996 8.572 L 13 8.572 L 13 8.01 L 12.007 8.01 Z");
  reload.setAttribute( "id", "reload" );
  reload.setAttribute( "transform", "matrix(1,0,0,1,86,0)" );
  document.getElementById( "s" ).appendChild( reload );
});

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

    * #reload {
        fill: ${stroke};
        stroke: ${mine_fill};
        stroke-width: 0;
        cursor: pointer;
    }

    * #reload:hover {
        stroke-width: 0.5;
    }


    .hex.focus > polygon {
        stroke: #b95c00;
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

EM_JS(void, init_hex, (float* points, float r, int id, float x, float y), {
  var group = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
  group.id = "h" + id;
  group.setAttribute( "transform", "matrix(1,0,0,1," + x + "," + y + ")" );
  group.setAttribute( "class", "hex" );
  document.getElementById( "s" ).appendChild( group );

  var polygon = document.createElementNS( "http://www.w3.org/2000/svg", "polygon" );
  var hexPoints = []; 
  for( var i = 0 ; i < 14; i ++ ) {
        hexPoints.push( HEAPF32[ (points + i * 4 ) >> 2 ] );
  }
  polygon.setAttribute( "points", hexPoints.join( "," ) );
  group.appendChild( polygon);

  var text = document.createElementNS( "http://www.w3.org/2000/svg", "text" );
  text.id = "t" + id;
  text.textContent = "?";
  text.setAttribute( "font-size", r );
  text.setAttribute( "text-anchor", "middle" );
  text.setAttribute( "dominant-baseline", "middle" );
  
  group.appendChild( text );
});

EM_JS(void, set_class, ( int id, const char* new_class_ ), {
    id = "h" + id;
    var svg = document.getElementById( 's' ); 
    var cell = document.getElementById( id );
    if( new_class_ != 0 ) {
        new_class = UTF8ToString( new_class_ );
        cell.setAttribute( "class", new_class );
    }
    svg.appendChild( cell );
});

EM_JS(void, add_class, ( int id, const char* new_class_ ), {
    id = "h" + id;
    var svg = document.getElementById( 's' ); 
    console.log( "add_class: ", id );
    var cell = document.getElementById( id );
    if( new_class_ != 0 ) {
        new_class = UTF8ToString( new_class_ );
        curr_class = cell.getAttribute( "class" ) || "";
        if( !curr_class.match( new_class ) ) {
            cell.setAttribute( "class", curr_class + " " + new_class );
        }
    }
    svg.removeChild( cell );
    svg.appendChild( cell );
});

EM_JS(void, remove_class, ( int id, const char* new_class_ ), {
    id = "h" + id;
    var svg = document.getElementById( 's' ); 
    var cell = document.getElementById( id );
    var reg = RegExp( " " + UTF8ToString( new_class_ ) );
    reg.global = true;

    if( new_class_ != 0 ) {
        curr_class = cell.getAttribute( "class" ) || "";
        cell.setAttribute( "class", curr_class.replace( reg, "" ) );
    }
    svg.removeChild( cell );
    svg.appendChild( cell );
});


EM_JS(void, set_cell_text, ( int id, const char* content ), {
    id = "t" + id;
    var text = document.getElementById( id );
    text.textContent = UTF8ToString( content );
});

void set_mine_count( int id, int mine_count ) {
    char* input;
    sprintf( input, "%d", mine_count );
    set_cell_text( id, input );
    free( input );
}

EM_BOOL cellFocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    Cell* cell = (Cell*) userData;
    printf( "focus: %d, neighbours:", cell->index );
    for(int i=0; i<6; i++) {
        if( cell->neighbours[ i ] >= 0 ) {
            add_class( cell->neighbours[ i ], "focus" );
            printf( " %d", cell->neighbours[ i ] );
        }
    }
    printf( "\n" );
    set_class( cell->index, NULL );
    return EM_TRUE;
}

EM_BOOL cellUnfocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    Cell* cell = (Cell*) userData;
    for(int i=0; i<6; i++) {
        if( cell->neighbours[ i ] >= 0 )
            remove_class( cell->neighbours[ i ], "focus" );
    }
    set_class( cell->index, NULL );
    return EM_TRUE;
}

EM_BOOL cellActivate( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    Cell* cell = (Cell*) userData;

    if( cell->is_mine == 0 ) {
        add_class( cell->index, "free" );
        set_mine_count( cell->index, cell->neighbourMines );
    } else {
        add_class( cell->index, "mine" );
        set_cell_text( cell->index, "*" );
    }

    return EM_TRUE;
}

HTMLDisplay::HTMLDisplay( Game* game ) {
    //emscripten_set_mouseup_callback( "reload", NULL, EM_TRUE, reload ); 
    make_svg();

    update_style( 
        1.0,  // transition speed
        "#fff", // stroke style
        0.5, // unknown cell stroke width
        "rgba(0,0,0,0)", // unknown cell fill
        1.0, // hover stroke width
        "#008900", // free fill
        "#ec0000", // mine fill
        "#b95c00" // flag fill
    );

    float cellRadius = 57.1593533487 / ( 2 * game->size + 1 );

    float cartesianConverters[] = {
        1.7320 * cellRadius,
        0.8660 * cellRadius,
        1.5 * cellRadius
    };

    float points[] = {
        0.0, cellRadius,
        0.866 * cellRadius, 0.5 * cellRadius,
        0.866 * cellRadius, -0.5 * cellRadius,
        0, -cellRadius,
        -0.866 * cellRadius, -0.5 * cellRadius,
        -0.866 * cellRadius, 0.5 * cellRadius,
        0.0, cellRadius
    };

    float x,y;
    for(int index=0; index<game->layout.cellCount; index++) {
        Cell* cell = game->layout.cells + index;
        int i = cell->position[ 0 ];
        int j = cell->position[ 1 ];
        x = 50 + i * cartesianConverters[ 0 ] + j * cartesianConverters[ 1 ];
        y = 50 + j * cartesianConverters[ 2 ];
        init_hex( points, cellRadius, index, x, y );
        char id[8];
        sprintf( id, "h%d", index );
        emscripten_set_mouseup_callback( id, ( void* ) cell, EM_TRUE, cellActivate ); 
        emscripten_set_mouseenter_callback( id, ( void* ) cell, EM_TRUE, cellFocus );
        emscripten_set_mouseleave_callback( id, ( void* ) cell, EM_TRUE, cellUnfocus );
        free(id);
    }

    make_reload();
}

int main() {
    ClassicHexagon classicHexagon;
    int seed = time(NULL);
    int size = 1;
    float mineRatio = 0.1;
    classicHexagon.setup_cells( size );
    classicHexagon.setup_neighbours();

    Game game( classicHexagon, seed, size, mineRatio );
    HTMLDisplay display( &game );
    
    return 0;
}
