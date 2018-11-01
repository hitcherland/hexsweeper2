#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <json.h>
#include <time.h>
#include <math.h>

struct activate_data {
    int id;
    int is_mine;
    int* mines;   
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
  text.textContent = "?";
  text.setAttribute( "font-size", r );
  text.setAttribute( "x", -r / 3 );
  text.setAttribute( "y",  r / 3 );
  
  group.appendChild( text);
});


EM_JS(void, update_class, ( int id, const char* new_class_ ), {
    id = "h" + id;
    new_class = UTF8ToString( new_class_ );
    document.getElementById( id ).setAttribute( "class", new_class );
});

EM_BOOL cellActivate( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    activate_data *data = (activate_data *) userData;
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

    if( data->is_mine == 0 )
        update_class( data->id, "hex free" );
    else
        update_class( data->id, "hex mine" );
    return EM_TRUE;
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

int flatten( int radius, int i, int j ) {
    int count = 0;
    for( int a=-radius; a<=radius; a++ ) {
        for( int b=-radius; b<=radius; b++ ) {
            if(a==i && b==j)
                return count;
            count++;
        }
    }
    return -1;
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
    for( int i = -radius; i<= radius; i++ ) {
        for( int j = -radius; j <= radius; j++) {
            if( abs( i + j ) > radius )
                continue;
            x = 50 + i * cartesianConverters[ 0 ] + j * cartesianConverters[ 1 ];
            y = 50 + j * cartesianConverters[ 2 ];
            make_hex( points, innerRadius, count, x, y );
            count++;
        }
    }
    int mine_count = round( count * mineRatio );
    int *mines = choose_in_range( count, mine_count );

    activate_data* datas[ count ]; 
    for( int i=0; i<count; i++) {
        activate_data* data = (activate_data *) malloc( sizeof(activate_data) );
        datas[ i ] = data;
        data->id = i;
        data->mines=mines;
        data->is_mine = 0;

        char *id = (char *) malloc( 8*sizeof(char) );
        sprintf( id, "h%d", i );

        emscripten_set_mouseup_callback(
            id,
            ( void* ) data,
            EM_TRUE,
            cellActivate
        );
        free( id );
    }

    for( int i = 0; i<mine_count; i++) {
        datas[ mines[ i ] ]->is_mine = 1;
    }
   
    return 0;
}
