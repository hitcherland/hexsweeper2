function getJson( path ) {
    var oReq = new XMLHttpRequest();
	var promise = new Promise( function( resolve, reject ) {
		var data;
		oReq.onload = function( e ) {
			data = JSON.parse( this.responseText );
			resolve( data );
		}
		oReq.onerror = reject
		oReq.open( "get", path );
		oReq.send();
		return data;
	});
	return promise;
}

function template( string, variables ) {
	return ( new Function(...Object.keys( variables ), 'return `' + string + '`' ) )( ...Object.values( variables ) );
}

function evaluate( string, variables ) {
	return eval( template( string, variables ) );
}

function cellTemplate( string, units, position ) {
	var cart_coords = Object.keys( position ).map( u => units[ u ].map( y => y * position[ u ] ) );
	var pos = cart_coords.reduce( (o,n) => { o.x += n[ 0 ]; o.y += n[ 1 ]; return o }, { x: 0, y: 0 } );
	return template( string, pos );
}

function getLayout( layout_name, outer_variables ) {
	if( outer_variables === undefined ) outer_variables = {};
	return getJson( "layouts/" + layout_name + ".json" ).then( function( data ) {
		var cell_types = data.cells;
		var units = data.units;
		var seed = data.layout.seed;
		var variables = Object.keys( data.variables ).reduce( (x,y) => { x[ y ] = outer_variables[ y ] || data.variables[ y ]; return x }, {} );
		var body = $( template( data.body, variables ) );
		body[ 0 ].innerHTML 

		var seedElements = seed.map( s => { 
			body[ 0 ].innerHTML += cellTemplate( cell_types[ s.cell ].html, units, s.position ) 
				var node = body[ 0 ].lastChild;
				$( node ).data( "position", s.position )
					.attr( "sweeper", "cell" );
			return { "cell": cell_types[ s.cell ], "position": s.position, "node": node };
		});

		var ungrownSeeds = seedElements;
		var takenPositions = seedElements.map( s => JSON.stringify( s.position ) );
		while( ungrownSeeds.length > 0 ) {
			var newSeeds = [].concat( ...ungrownSeeds.map( c => {
				return c.cell.neighbours.map( n => {
					var p = {};
					Object.keys( c.position ).forEach( x => { p[x] = (n.position[ x ]||0) + c.position[ x ] } );
					var pS = JSON.stringify( p );
					if( takenPositions.indexOf( pS ) >= 0 ) {
						return undefined;
					}
					var passes_rule = evaluate( data.layout.rule, {...variables, ...p} );
					if( ! passes_rule ) {
						return undefined;
					}

					body[ 0 ].innerHTML += cellTemplate( cell_types[ n.type ].html, units, p ) 
					takenPositions.push( pS );
					var node = body[ 0 ].lastChild;
					$( node ).data( "position", p )
						.attr( "sweeper", "cell" );
					return { "cell": cell_types[ n.type ], "position": p, "node": node };
				})
			}));
			ungrownSeeds = newSeeds.filter( x => x !==undefined );;
		}

		return body[ 0 ];
	}).catch( function( data ) {
		console.log( "problem:", data, arguments );
	});
}
