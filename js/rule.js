function getRule( rule_name, outer_variables ) {
	if( outer_variables === undefined ) outer_variables = {};
	return getJson( "rules/" + rule_name + ".json" ).then( function( data ) {
		var Cell = function( element, position ) {
			var variables = {};
			Object.keys( this.__proto__.variables.private ).forEach( k => {
				if( this.__proto__.variables.public[ k ] == "bool" ) {
					variables[ k ] = false;
				} else if( this.__proto__.variables.public[ k ] == "array" ) {
					variables[ k ] = [];
				}
			});

			Object.keys( this.__proto__.variables.public ).forEach( k => {
				if( this.__proto__.variables.public[ k ] instanceof Array ) {
					variables[ k ] = this.__proto__.variables.public[ k ][ 0 ] || undefined,
					Object.defineProperty( this, k, {
						get: function() {
							return variables[ k ];
						},
						set: function( val ) {
							if( this.__proto__.variables.public[ k ].indexOf( val ) >= 0 ) {
								variables[ k ] = val;
							} else {
								throw "Invalid value '" + val + "', should be one of '" + this.__proto__.variables.public[ k ].join( ', ' ) + "'";
							}
						}
					});
				} else if( this.__proto__.variables.public[ k ] == "position" ) {
					variables[ k ] = position;
				} else if( this.__proto__.variables.public[ k ] == "array" ) {
					variables[ k ] = [];
				}
			});
		};
		Cell.prototype = {
			variables: data.cell.variables,
		};

		var output = {
			"Cell": Cell,
			"variables": data.global.variables,
		};

		return output;
	}).catch( function( data ) {
		console.log( "problem:", data, arguments );
	});
}
