#include <html/cell.h>

EM_JS(void, add_class, ( int id, const char* new_class_ ), {
    id = "h" + id;
    var svg = document.getElementById( 's' ); 
    console.log( "add class", id );
    var cell = document.getElementById( id );
    if( new_class_ != 0 ) {
        new_class = UTF8ToString( new_class_ );
        old_class = cell.getAttribute( "class" );
        cell.setAttribute( "class", old_class + " " + new_class );
    }
    svg.appendChild( cell );
});

EM_JS(void, remove_class, ( int id, const char* class_ ), {
    id = "h" + id;
    var svg = document.getElementById( 's' ); 
    console.log( "remove class", id );
    var cell = document.getElementById( id );
    if( class_ != 0 ) {
        bad_class = UTF8ToString( class_ );
        new_class = cell.getAttribute( "class" ).replace( bad_class, "" );
        cell.setAttribute( "class", old_class );
    }
    svg.appendChild( cell );
});

EM_JS(void, make_hex, (int id, float x, float y, float cellRadius), {
    var hexPoints = [
        0.0, 1,
        0.866, 0.5,
        0.866, -0.5,
        0, -1,
        -0.866, -0.5,
        -0.866, 0.5,
        0.0, 1
    ].map( x => x * cellRadius );

    var group = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
    group.id = "h" + id;
    group.setAttribute( "transform", "matrix(1,0,0,1," + x + "," + y + ")" );
    group.setAttribute( "class", "hex" );
    document.getElementById( "s" ).appendChild( group );

    var polygon = document.createElementNS( "http://www.w3.org/2000/svg", "polygon" );
    polygon.setAttribute( "points", hexPoints.join( "," ) );
    group.appendChild( polygon);

    var text = document.createElementNS( "http://www.w3.org/2000/svg", "text" );
    text.id = "t" + id;
    text.textContent = "?";
    text.setAttribute( "font-size", cellRadius );
    text.setAttribute( "x", -cellRadius / 3 );
    text.setAttribute( "y",  cellRadius / 3 );

    group.appendChild( text);
});

EM_BOOL cellActivate( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
	HTMLCell *cell = (HTMLCell* ) userData;
    printf( "pre-activate: %p, %p, %d\n", userData, cell, cell->id );
    cell->activate();
    return EM_TRUE;
}

EM_JS(void, set_cell_text, ( int id, const char* str_ ), {
    id = "t" + id;
    var text = document.getElementById( id );
    var textContent = UTF8ToString( str_ );
    text.textContent = textContent;
});

EM_BOOL cellFocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    HTMLCell* cell = (HTMLCell*) userData;
    printf( "focused: %d\n", cell->id );
    for(int i=0; i<6; i++) {
        if( cell->neighbours[ i ] != NULL )
            cell->neighbours[ i ]->addClass( "neighbour" );
    }
    return EM_TRUE;
}

EM_BOOL cellUnfocus( int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData ) {
    HTMLCell* cell = (HTMLCell*) userData;
    for(int i=0; i<6; i++) {
        if( cell->neighbours[ i ] != NULL )
            cell->neighbours[ i ]->removeClass( "neighbour" );
    }
    return EM_TRUE;
}

HTMLCell::HTMLCell() {
    id = -1;
    isMine = -1;
    mineCount = -1;
    activated = -1;
}

HTMLCell::HTMLCell( int index, float x, float y, float radius ) {
    init( index, x, y, radius );
}

void HTMLCell::init( int index, float x, float y, float radius ) {
    make_hex( index, x, y, radius );

    char str_id[ 8 ];
    sprintf( str_id, "h%d", index );
    emscripten_set_mouseup_callback( str_id, ( void* ) this, EM_TRUE, cellActivate ); 
    free( str_id );


    id = index;
    isMine = 0;
    mineCount = 0;
    activated = 0;
    for( int i=0; i<6; i++) neighbours[ i ] = NULL;

    //emscripten_set_mouseenter_callback( str_id, ( void* ) this, EM_TRUE, cellFocus );
    //emscripten_set_mouseleave_callback( str_id, ( void* ) this, EM_TRUE, cellUnfocus );
}

void HTMLCell::activate() {
    printf( "cell: %d (%p)\n", id, this );
	if( activated > 0 ) return;
	activated = 1;

    if( isMine == 0 ) {
        showMineCount();
        addClass( "free" );
		//simple_solve( cell );
    } else {
        addClass( "mine" );
    }
};

void HTMLCell::addClass( const char* string) {
    add_class( id, string );
}

void HTMLCell::removeClass( const char* string) {
    remove_class( id, string );
}

void HTMLCell::showMineCount() {
    if( mineCount == 0 ) {
        set_cell_text( id, "" );
    } else {
        char *str_minecount;
        sprintf( str_minecount, "%d", mineCount );
        set_cell_text( id, str_minecount );
    }
}
