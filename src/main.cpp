#include <emscripten.h>
#include <emscripten/html5.h>
#include <json.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct activate_data {
    int id;
    int is_mine;
    int* mines;   
    int* neighbours;
    int mine_count;
    int lock;
};

struct Game {
    int radius;
    int count;
    float mineRatio;
    int mine_count;
    int* mines;
    int* rows;
    activate_data* datas;
} game;

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

EM_JS(void, make_hex, (float* points, float r, int id, float x, float y), {
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
    activate_data *data = (activate_data *) userData;
    for(int i=0; i<6; i++) {
        if( data-> neighbours[ i ] >= 0 ) {
            add_class( data->neighbours[ i ], "focus" );
        }
    }
    set_class( data->id, NULL );
    return EM_TRUE;
}

EM_BOOL cellUnfocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    activate_data *data = (activate_data *) userData;
    for(int i=0; i<6; i++) {
        if( data->neighbours[ i ] >= 0 )
            remove_class( data->neighbours[ i ], "focus" );
    }
    set_class( data->id, NULL );
    return EM_TRUE;
}

EM_BOOL cellActivate( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    activate_data *data = (activate_data *) userData;
    data->lock = 1;
    if( data->is_mine == -1 ) {
        int i=0;
        while( data->mines[ i ] != NULL ) {
            if( data->mines[ i ] == data->id ) {
                data->is_mine = 1;
                break;
            }
            i++;
        }
        if( data->is_mine != 1 ) 
            data->is_mine = 0;
    }

    if( data->is_mine == 0 ) {
        add_class( data->id, "free" );
        set_mine_count( data->id, data->mine_count );
    } else {
        add_class( data->id, "mine" );
        set_cell_text( data->id, "*" );
    }

    return EM_TRUE;
}

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

void start_game( Game *game ) {
    if( game->count != 0 ) {
        for( int i=0; i<game->count; i++) {
            free( game->datas[ i ].neighbours );
        }
        free( game->datas );
        free( game->mines );
    }

    game->mines = choose_in_range( game->count, game->mine_count );
    game->datas = (activate_data*) malloc( game->count * sizeof( activate_data ) );

    int row = 0;
    int prev_row_length = -1;
    int row_length = game->rows[ 0 ];
    int next_row_length = game->rows[ 1 ] - game->rows[ 0 ];
    for( int i=0; i<game->count; i++) {
        if( i == game->rows[ row ] ) {
            row++;
            prev_row_length = row_length;
            row_length = game->rows[ row ] - game->rows[ row - 1 ];
            next_row_length = row < 2 * game->radius + 1 ? game->rows[ row + 1 ] - game->rows[ row ] : -1;
        }

        activate_data data = game->datas[ i ];
        data.id = i;
        data.mines = game->mines;
        data.is_mine = 0;
        data.neighbours = (int *) malloc( 6 * sizeof(int) );
        
        int top = game->rows[ row ] - row_length == i && row >= game->radius;
        int top_right = row == 2 * game->radius;
        int bottom_right = game->rows[ row ] == i + 1 && row >= game->radius;
        int bottom = game->rows[ row ] == i + 1 && row <= game->radius;
        int bottom_left = row == 0;
        int top_left = game->rows[ row ] - row_length == i && row <= game->radius;

        data.neighbours[ 0 ] = !( top || top_right ) ? i + fmax( row_length, next_row_length ) - 1 : -1;
        data.neighbours[ 1 ] = !( top_right || bottom_right ) ? i + fmax( row_length, next_row_length ) : -1;
        data.neighbours[ 2 ] = !( bottom_right || bottom ) ? i + 1 : -1;
        data.neighbours[ 3 ] = !( bottom || bottom_left ) ? i - fmax( row_length, prev_row_length ) + 1: -1;
        data.neighbours[ 4 ] = !( bottom_left || top_left ) ? i - fmax( row_length, prev_row_length ): -1;;
        data.neighbours[ 5 ] = !( top || top_left ) ? i - 1: -1;;
    
        data.mine_count = 0;
        data.lock = 0;

        char *id = (char *) malloc( 8*sizeof(char) );
        sprintf( id, "h%d", i );

        emscripten_set_mouseup_callback( id, ( void* ) &data, EM_TRUE, cellActivate ); 
        emscripten_set_mouseenter_callback( id, ( void* ) &data, EM_TRUE, cellFocus );
        emscripten_set_mouseleave_callback( id, ( void* ) &data, EM_TRUE, cellUnfocus );
        free( id );
    }


    for( int i=0; i<game->mine_count; i++) {
        game->datas[ game->mines[ i ] ].is_mine = 1;
        for( int j=0; j<6; j++) {
            game->datas[ game->datas[ game->mines[ i ] ].neighbours[ j ] ].mine_count+=1;
        }
    }
}

EM_BOOL reload( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    start_game( &game );
    return EM_FALSE;
}

void read_json( const char* filename ) {
    char *json_string;
    FILE *f = fopen( filename, "rb" );
    struct json_object *json, *tmp;
    long length;
    
    if( f ) {
        fseek( f, 0, SEEK_END );
        length = ftell( f );
        fseek( f, 0, SEEK_SET);
        json_string = (char *) malloc( length );
        if( json_string ) {
            fread( json_string, 1, length, f );
        }
        fclose( f );
    } else {
        printf( "%s is not a file\n", filename );
    }

    if( json_string ) {
        json = json_tokener_parse( json_string );
        tmp = json_object_object_get( json, "background" );
        if( tmp ) {
            printf( "%s\n", json_object_get_string( tmp ) );
        }
    } else {
        printf( "tried to read %s, but it failed?\n", filename );
    }
    free( json_string );
}

int main() {
    game.count = 0;

    srand(time(NULL));
    make_svg();
    make_reload();
    emscripten_set_mouseup_callback( "reload", NULL, EM_TRUE, reload ); 

    //read_json( "/styles/dark.json" );
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

    int radius = 3;
    float mineRatio = 0.1;
    float innerRadius = 57.1593533487 / ( 2 * radius + 1 );

    float cartesianConverters[] = {
        1.7320 * innerRadius,
        0.8660 * innerRadius,
        1.5 * innerRadius
    };

    float points[] = {
        0.0, innerRadius,
        0.866 * innerRadius, 0.5 * innerRadius,
        0.866 * innerRadius, -0.5 * innerRadius,
        0, -innerRadius,
        -0.866 * innerRadius, -0.5 * innerRadius,
        -0.866 * innerRadius, 0.5 * innerRadius,
        0.0, innerRadius
    };

    float x, y;
    int count = 0;
    int rows[ 2 * radius + 1 ];
    for( int i = -radius; i<= radius; i++ ) {
        for( int j = -radius; j <= radius; j++) {
            if( abs( i + j ) > radius )
                continue;
            x = 50 + i * cartesianConverters[ 0 ] + j * cartesianConverters[ 1 ];
            y = 50 + j * cartesianConverters[ 2 ];
            make_hex( points, innerRadius, count, x, y );
            count++;
        }
        rows[ i + radius ] = count;
    }

    game.radius = radius;
    game.count = count;
    game.mineRatio = mineRatio;
    game.mine_count = round( count * mineRatio );
    game.rows = rows;
    start_game( &game );

    return 0;
}
