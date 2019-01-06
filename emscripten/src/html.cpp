#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include "game.h"

EM_JS(void, create_cell, (int h, int j, int radius), {
	var cell = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
	var polygon = document.createElementNS( "http://www.w3.org/2000/svg", "polygon" );
    cell.appendChild( polygon );

	var cell_radius = 57.1593533487 / ( 2.0 * radius + 1.0 );

    var x = h * 1.7320 * cell_radius + j * 0.8660 * cell_radius;
    var y = j * 1.5 * cell_radius;

	var points = [
		0.0, cell_radius,
		0.866 * cell_radius, 0.5 * cell_radius,
		0.866 * cell_radius, -0.5 * cell_radius,
		0 * cell_radius, -cell_radius,
		-0.866 * cell_radius, -0.5 * cell_radius,
		-0.866 * cell_radius, 0.5 * cell_radius,
		0.0, cell_radius
	].join( "," );
	

	$( polygon )
		.attr({
			"data-h": h,
			"data-j": j,
			"points": points
		})
        .appendTo( cell );
    $( cell )
        .attr({
			"id": `cell_${h}_${j}`,
			"transform": `matrix(1,0,0,1,${x},${y})`,
            "style": `font-size:${cell_radius}px`
        })
        .appendTo( '#game' );
});

EM_JS(void, toggle_cell_class, ( int h, int j, const char* class_name ), {
    $( `#cell_${h}_${j} > polygon` ).toggleClass( UTF8ToString( class_name ) );
});

EM_JS(void, toggle_id_class, (const char* id, const char* class_name ), {
    $( "#" + UTF8ToString( id ) ).toggleClass( UTF8ToString( class_name ) );
});

EM_JS(void, add_id_class, (const char* id, const char* class_name ), {
    $( "#" + UTF8ToString( id ) ).addClass( UTF8ToString( class_name ) );
});

EM_JS(void, remove_id_class, (const char* id, const char* class_name ), {
    $( "#" + UTF8ToString( id ) ).removeClass( UTF8ToString( class_name ) );
});

EM_JS(void, remove_groups, (), {
    $( '#game > g' ).remove();
});

EM_JS(void, add_mine_count, (int h, int j, int mine_count ), {
	var text = document.createElementNS( "http://www.w3.org/2000/svg", "text" );
    $( text )
        .text( mine_count >= 0 ? mine_count : "!" )
        .appendTo( `#cell_${h}_${j}` )
});

EM_BOOL reveal_on_mousedown( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    std::cout << "reveal (down): " << e->timestamp << std::endl;
    reveal_timestamp = e->timestamp;
    return 0;
}

EM_BOOL reveal_on_mouseup( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    double dt = e->timestamp - reveal_timestamp;
    if( dt > LONG_PRESS ) {
        emscripten_vibrate( 100 );
        if( click_mode != REVEAL ) {
            click_mode = REVEAL; 
            remove_id_class( "flag", "selected" );
            add_id_class( "reveal", "selected" );
        } else {
            click_mode = SELECT;
            remove_id_class( "flag", "selected" );
            remove_id_class( "reveal", "selected" );
        }
    } else if( click_mode == FLAG ) {
        remove_id_class( "flag", "selected" );
        add_id_class( "reveal", "selected" );
        click_mode = REVEAL; 
    } else if( focused_cell != NULL && click_mode == SELECT) {
        emscripten_vibrate( 40 );
        reveal( focused_cell );
    }

    std::cout << "reveal (up): " << e->timestamp << " -> " << dt << " vs. " << LONG_PRESS << " means " << click_mode << std::endl;
    return 0;
}

EM_BOOL flag_on_mousedown( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    flag_timestamp = e->timestamp;
    return 0;
}

EM_BOOL flag_on_mouseup( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    double dt = e->timestamp - flag_timestamp;
    if( dt > LONG_PRESS ) {
        emscripten_vibrate( 100 );
        if( click_mode != FLAG ) {
            add_id_class( "flag", "selected" );
            remove_id_class( "reveal", "selected" );
            click_mode = FLAG; 
        } else {
            click_mode = SELECT;
            remove_id_class( "flag", "selected" );
            remove_id_class( "reveal", "selected" );
        }
    } else if( click_mode == REVEAL ) {
        add_id_class( "flag", "selected" );
        remove_id_class( "reveal", "selected" );
        click_mode = FLAG; 
    } else if( focused_cell != NULL && click_mode == SELECT) {
        emscripten_vibrate( 40 );
        flag( *focused_cell );
    }
    return 0;
}

EM_BOOL fullscreen_on_click( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    emscripten_vibrate( 20 );
    if( fullscreen ) 
        emscripten_exit_fullscreen();
    else
        emscripten_request_fullscreen( "layout", false );
    fullscreen = ! fullscreen;
    return 0;
}

EM_BOOL cell_on_click( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    emscripten_vibrate( 3 );
    int h = EM_ASM_INT({
        return $( document.elementFromPoint( $0, $1 ) ).data( 'h' );
    }, e->clientX, e->clientY);

    int j = EM_ASM_INT({
        return $( document.elementFromPoint( $0, $1 ) ).data( 'j' );
    }, e->clientX, e->clientY);

    Cell *cell = find_cell( h, j );

    focused_cell = NULL;
    if( cell->revealed ) {
        return 0;
    } else if( click_mode == REVEAL ) {
        reveal( cell );
    } else if( click_mode == FLAG ) {
        flag( *cell );
    } else if( click_mode == SELECT ) {
        focused_cell = cell;
    }
	return 0;
}

EM_BOOL cell_on_click_setup( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    emscripten_vibrate( 100 );
    int h = EM_ASM_INT({
        return $( document.elementFromPoint( $0, $1 ) ).data( 'h' );
    }, e->clientX, e->clientY);

    int j = EM_ASM_INT({
        return $( document.elementFromPoint( $0, $1 ) ).data( 'j' );
    }, e->clientX, e->clientY);

    Cell *cell = find_cell( h, j );
	place_mines( 0.3, cell );
    cell = find_cell( h, j );
    reveal( cell );
	return 0;
}

EM_BOOL hide_options( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    emscripten_vibrate( 10 );
    EM_ASM({
        $( '#options' ).css( "display", "none");
    });
    return 0;
}

EM_BOOL show_options( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
    emscripten_vibrate( 10 );
    EM_ASM({
        $( '#options' ).css( "display", "grid");
    });
    return 0;
}

void reset_cell( Cell &cell ) {
    std::stringstream id_stream;
    id_stream << "cell_" << cell.h << "_" << cell.j; 
    std::string id_str = id_stream.str();

    emscripten_set_click_callback( id_str.c_str(), 0, 0, NULL);
    emscripten_set_click_callback( id_str.c_str(), (void*) &cell, 1, cell_on_click);
}

void display_clean( int r ) {
    remove_groups();
}

void display_flag( Cell &cell ) {
    toggle_cell_class( cell.h, cell.j, "flagged" );
}

void display_reveal( Cell *cell ) {
    toggle_cell_class( cell->h, cell->j, cell->is_mine ? "mine" : "free" );
    if( !cell->is_mine ) {
        add_mine_count( cell->h, cell->j, cell->mine_count );
    } else {
        add_mine_count( cell->h, cell->j, -1 );
    }
}

void display_create_cell( Cell &cell ) {
    std::stringstream id_stream;
    id_stream << "cell_" << cell.h << "_" << cell.j; 
    std::string id_str = id_stream.str();

    float radius = EM_ASM_DOUBLE({
        return $( '[name=radius]' ).val();
    }, NULL);

    create_cell( cell.h, cell.j, radius );
    emscripten_set_click_callback( id_str.c_str(), (void*) &cell, 1, cell_on_click_setup);
}

extern "C" {
    void update_radius( int radius ) {
        generate_layout( radius );
    } 
}

int main( int argc, char* argv[] ) {
    std::cout << "//SWEEPER\\\\";
    for( int i=0; i<argc; i++ ) {
        std::string str( argv[ i ] );
        if( str.find( "radius" ) != std::string::npos ) {
            radius = std::stoi( str.substr( 7 ) );
        }

        if( str.find( "mine_ratio" ) != std::string::npos ) {
            mine_ratio = std::stof( str.substr( 11 ) );
        }
    }

    if( radius < 1 ) {
        radius = 1;
        std::cerr << "radius less than 1 is an invalid, setting to 1" << std::endl;
    }

    if( mine_ratio < 0 || mine_ratio > 1 ) {
        mine_ratio = 0.3;
        std::cerr << "mine_ratio should be between 0 and 1, setting to 0.3" << std::endl;
    }

    emscripten_set_click_callback( "fullscreen", 0, 0, fullscreen_on_click);
    emscripten_set_mousedown_callback( "reveal", 0, 0, reveal_on_mousedown);
    emscripten_set_mouseup_callback( "reveal", 0, 0, reveal_on_mouseup);

    emscripten_set_mousedown_callback( "flag", 0, 0, flag_on_mousedown);
    emscripten_set_mouseup_callback( "flag", 0, 0, flag_on_mouseup);

    emscripten_set_click_callback( "hide_options", 0, 0, hide_options);
    emscripten_set_click_callback( "show_options", 0, 0, show_options);

    EM_ASM({
        return $( '[name="radius"]' ).val( $0 );
    }, radius);

    EM_ASM({
        return $( '[name="mine_ratio"]' ).val( $0 );
    }, std::round( mine_ratio * 100 ) / 100 );

    std::cout << "radius=" << radius << std::endl;

    setup( radius );
    return 0;
}
