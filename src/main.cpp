#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <json.h>
#include <stddef.h>

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
    }

    . { 
        -webkit-user-select: none;  /* Chrome all / Safari all */
        -moz-user-select: none;     /* Firefox all */
        -ms-user-select: none;      /* IE 10+ */
        user-select: none;          /* Likely future */ 
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

EM_JS(void, make_hex, (float* points, float r, char* id_, float x, float y), {
  var group = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
  group.id = UTF8ToString( id_ );
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


EM_JS(void, update_class, ( char* id_, const char* new_class_ ), {
    id = UTF8ToString( id_ );
    new_class = UTF8ToString( new_class_ );
    document.getElementById( id ).setAttribute( "class", new_class );
});

EM_BOOL cellActivate( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    char *id = (char *) userData;
    printf( "click %s\n", id );
    update_class( id, "hex free" );
    return EM_TRUE;
}

void read_json( const char* filename ) {
    char *json_string;
    FILE *f = fopen( filename, "rb" );
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
        printf( "%s\n", json_string );
    } else {
        printf( "tried to read %s, but it failed?\n", filename );
    }
}

int main() {
    try {
    make_svg();
    read_json( "/styles/dark.json" );
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
    for( int i = -radius; i<= radius; i++ ) {
        for( int j = -radius; j <= radius; j++) {
            if( abs( i + j ) > radius )
                continue;
            x = 50 + i * cartesianConverters[ 0 ] + j * cartesianConverters[ 1 ];
            y = 50 + j * cartesianConverters[ 2 ];
            char* id = (char *) malloc( 12 * sizeof(char) );
            sprintf( id, "h%d_%d", i, j );
            make_hex( points, innerRadius, id, x, y );
            emscripten_set_mouseup_callback(
                id,
                ( void* ) id,
                EM_TRUE,
                cellActivate
            );

        }
    }
    return 0;
    } catch( ... ) {
        printf( "seems like a problem came up?\n" );
    }
}
