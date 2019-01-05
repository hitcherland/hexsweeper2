#include <emscripten.h>
#include <emscripten/html5.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

typedef struct Cell {
	int h;
	int j;
	bool is_mine;
	int mine_count;
} Cell;

Cell* cells;

EM_JS(void, create_cell, (int h, int j, float x, float y, const char* points), {
	var points = UTF8ToString( points );
	var polygon = document.createElementNS( "http://www.w3.org/2000/svg", "polygon" );
	
	$( polygon )
		.appendTo( "#game" )
		.attr({
			"id": `cell_${h}_${j}`,
			"data-h": h,
			"data-j": j,
			"transform": `matrix(1,0,0,1,${x},${y})`,
			"points": points
		});
});

EM_JS(void, create_options_page, (), {
	var full_page = $( '<div id="options"></div>' ).appendTo( document.body );
});


EM_BOOL cell_on_click( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
	if( eventType == EMSCRIPTEN_EVENT_CLICK ) {
		Cell* cell = (Cell*) userData;
		std::cout << "hello clicker: " << cell->h << "," << cell->j << " --> " << cell->is_mine << std::endl;
	}
	return 0;
}

void init_game( int radius, float mine_ratio ) {
	cells = (Cell*) malloc( ( 3 * ( radius * radius + 1 ) + 1 ) * sizeof( Cell ) );
	int count = 3 * radius * ( radius + 1 ) + 1;
	int mine_count = mine_ratio * count;
	int indexes[ count ];
	for( int i=0; i<count; i++)
		indexes[ i ] = i;
	std::random_shuffle(indexes, indexes+count);

	int index = 0;
	for( int h=-radius; h<=radius; h++) {
		for( int j=-radius; j<=radius; j++) {
			if( std::abs( h + j ) <= radius && std::abs( h ) <= radius && std::abs( j ) <= radius ) {
				std::stringstream id_stream;
				id_stream <<"cell_" << h << "_" << j; 
				std::string id_str = id_stream.str();

				cells[ index ].h = h;
				cells[ index ].j = j;
				cells[ index ].is_mine = false;
				cells[ index ].mine_count = 0;
				emscripten_set_click_callback( id_str.c_str(), 0, 0, NULL);
				emscripten_set_click_callback( id_str.c_str(), (void*) &cells[ index ], 1, cell_on_click);

				for( int i=0; i<mine_count; i++) {
					if(index == indexes[ i ]) {
						cells[ index ].is_mine = true;
						break;
					}
				}
				index += 1;
			}
		}
	}
}

EM_BOOL cell_on_click_setup( int eventType, const EmscriptenMouseEvent *e, void *userData ) {
	int radius = *((int*) userData);
	init_game( radius, 0.3 );
	std::cout << "setup: " << "test" << std::endl;
	return 0;
}



void generate_dom( int radius ) {
	float cellRadius = 57.1593533487 / ( 2.0 * radius + 1.0 );

	float hexPoints[] = {
		0.0, 1,
		0.866, 0.5,
		0.866, -0.5,
		0, -1,
		-0.866, -0.5,
		-0.866, 0.5,
		0.0, 1
	};

	std::stringstream ss;
	for( int i=0; i<14; i++ ) {
		if( i != 0 )
			ss << ",";
		ss << hexPoints[ i ] * cellRadius;
	}
	std::string tmp = ss.str();
	const char* points = tmp.c_str();



	for( int h=-radius; h<=radius; h++) {
		for( int j=-radius; j<=radius; j++) {
			if( std::abs( h + j ) <= radius && std::abs( h ) <= radius && std::abs( j ) <= radius ) {
				float x = h * 1.7320 * cellRadius + j * 0.8660 * cellRadius;
				float y = j * 1.5 * cellRadius;
				create_cell( h, j, x, y, points );

				std::stringstream id_stream;
				id_stream <<"cell_" << h << "_" << j; 
				std::string id_str = id_stream.str();

				emscripten_set_click_callback( id_str.c_str(), (void*) &radius, 1, cell_on_click_setup);
			}
		}
	}
}

int main( int argc, char* argv[] ) {
	// query string 
	int radius = 4;
	std::cout << "Running Sweeper" << std::endl;
	for( int i=0; i<argc; i++ ) {
		std::string str( argv[ i ] );
		if( str.find( "radius" ) != std::string::npos ) {
			radius = std::stoi( str.substr( 7 ) );
		}
	}

	if( radius == 0 ) {
		radius = 1;
		std::cerr << "radius=0 is an invalid radius, setting to 1" << std::endl;
	}


	std::cout << "radius=" << radius << std::endl;
	generate_dom( radius );
	create_options_page();

	return 0;
}
