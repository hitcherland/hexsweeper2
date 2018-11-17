#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <json.h>
#include <time.h>
#include <math.h>

class Cell {
	public:
		int id;
		int is_mine;
		Cell** neighbours;
		int mine_count;
		int activated;
};

struct ActivateData {
    int id;
    int is_mine;
    int* mines;   
    int* neighbours;
    int mine_count;
    int lock;
	ActivateData** datas;
};

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
  text.setAttribute( "x", -r / 3 );
  text.setAttribute( "y",  r / 3 );
  
  group.appendChild( text);
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

EM_JS(void, update_class, ( int id, const char* new_class_ ), {
    id = "h" + id;
    var svg = document.getElementById( 's' ); 
    var cell = document.getElementById( id );
    if( new_class_ != 0 ) {
        new_class = UTF8ToString( new_class_ );
        cell.setAttribute( "class", new_class );
    }
    svg.appendChild( cell );
});

EM_JS(void, set_mine_count, ( int id, int mine_count ), {
    id = "t" + id;
    var text = document.getElementById( id );
	if( mine_count == 0 ) {
		text.textContent = " ";
	} else {
		text.textContent = mine_count;
	}
});


EM_BOOL cellFocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    ActivateData *data = (ActivateData *) userData;
    for(int i=0; i<6; i++) {
        if( data-> neighbours[ i ] >= 0 )
            update_class( data->neighbours[ i ], "hex focus" );
    }
    printf( "\n" );
    printf( "\n" );
    return EM_TRUE;
}

EM_BOOL cellUnfocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    ActivateData *data = (ActivateData *) userData;
    for(int i=0; i<6; i++) {
        if( data->neighbours[ i ] >= 0 )
            update_class( data->neighbours[ i ], "hex" );
    }
    update_class( data->id, NULL );
    return EM_TRUE;
}

void simple_solve( Cell* cell );
EM_BOOL cellActivate( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
	Cell *cell = (Cell* ) userData;
	if( cell->activated > 0 ) {
		return EM_TRUE;
	}

	cell->activated = 1;

    if( cell->is_mine == 0 ) {
        set_mine_count( cell->id, cell->mine_count );
        update_class( cell->id, "hex free" );
		simple_solve( cell );
    } else
        update_class( cell->id, "hex mine" );

    return EM_TRUE;
}

void simple_solve( Cell* cell ) {
	if( cell->mine_count == 0 ) {
		for( int i=0; i<6; i++) {
			Cell* neighbour = cell->neighbours[ i ];
			if( neighbour != NULL ) {
				cellActivate( 0, NULL, (void*) neighbour );
			}
		}
	}
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

int main() {
    srand(time(NULL));
    make_svg();
    //read_json( "/styles/dark.json" );
    int radius = get_outer_radius();
	if (radius < 0 ) {
		radius = 3;
	} 
    float mineRatio = 0.1;
    float innerRadius = 57.1593533487 / ( 2 * radius + 1 );

    update_style( 
        1.0,  // transition speed
        "#fff", // stroke style
        0.1 * innerRadius, // unknown cell stroke width
        "rgba(0,0,0,0)", // unknown cell fill
        0.2 * innerRadius, // hover stroke width
        "#008900", // free fill
        "#ec0000", // mine fill
        "#b95c00" // flag fill
    );

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
    int mine_count = round( count * mineRatio );
    int *mines = choose_in_range( count, mine_count );

    ActivateData* datas[ count ]; 
	Cell* cells[ count ];

    int row = 0;
    int prev_row_length = -1;
    int row_length = rows[ 0 ];
    int next_row_length = rows[ 1 ] - rows[ 0 ];
    for( int i=0; i<count; i++) {
		Cell* cell = ( Cell* ) malloc( sizeof( Cell ) );
		cells[ i ] = cell;
		cell->id = i;
		cell->is_mine = 0;
		cell->mine_count = 0;
		cell->neighbours = (Cell**) malloc( 6 * sizeof( Cell* ) );
		cell->activated = 0;
	}

    for( int i=0; i<count; i++) {
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

		Cell* cell = cells[ i ];
        cell->neighbours[ 0 ] = !( top || top_right ) ? cells[ int(  i + fmax( row_length, next_row_length ) - 1   ) ]: NULL;
        cell->neighbours[ 1 ] = !( top_right || bottom_right ) ? cells[ int(  i + fmax( row_length, next_row_length )   ) ]: NULL;
        cell->neighbours[ 2 ] = !( bottom_right || bottom ) ? cells[ int(  i + 1   ) ]: NULL;
        cell->neighbours[ 3 ] = !( bottom || bottom_left ) ? cells[ int(  i - fmax( row_length, prev_row_length ) + 1  ) ]: NULL;
        cell->neighbours[ 4 ] = !( bottom_left || top_left ) ? cells[ int(  i - fmax( row_length, prev_row_length )  ) ]: NULL;
        cell->neighbours[ 5 ] = !( top || top_left ) ? cells[ int(  i - 1  ) ]: NULL;

        char *id = (char *) malloc( 8*sizeof(char) );
        sprintf( id, "h%d", i );

        emscripten_set_mouseup_callback( id, ( void* ) cell, EM_TRUE, cellActivate ); 
        //emscripten_set_mouseenter_callback( id, ( void* ) data, EM_TRUE, cellFocus );
        //emscripten_set_mouseleave_callback( id, ( void* ) data, EM_TRUE, cellUnfocus );
        free( id );
    }

    for( int i=0; i<mine_count; i++) {
		cells[ mines[ i ] ]->is_mine = 1;
        for( int j=0; j<6; j++) {
			Cell* neighbour = cells[ mines[ i ] ]->neighbours[ j ];
			if( neighbour != NULL ) {
				neighbour->mine_count++;
			}
        }
    }

    return 0;
}