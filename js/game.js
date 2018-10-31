function distance( pointers ) {
    var Ax = pointers[ 0 ].offsetX;
    var Ay = pointers[ 0 ].offsetY;
    var Bx = pointers[ 1 ].offsetX;
    var By = pointers[ 1 ].offsetY;
    return Math.pow( Ax - Bx, 2 ) + Math.pow( Ay - By, 2 );
}

class SimpleHexSweeper {
    constructor( snap, outerRadius, mineRatio ) {
        this.snap = snap;
        this.mineRatio = mineRatio;
        this.outerRadius = outerRadius;

        this.innerRadius = 0;
        this.hexes = Snap.set();
        this.mines = Snap.set();
        this.flags = Snap.set();
        this.frees = Snap.set();

        this._j = [];
        this._h = 0;

        this.rebuild();
        this.exes
        this.hexes.forEach( x => x.data( 'game', this ) );
        this.hexes.forEach(
            hex => hex.mouseup( this.initialise )
        );
        this.first = true;

        this.zoomLevel = 1;
        this.center = [ 50, 50 ];
        this.panStart = undefined;
        this.zoomStart = undefined;
    
        $( this.snap.node ).data( 'game', this );
        $( this.snap.node ).hammer();
        $( this.snap.node ).data( 'hammer' ).get( 'pinch' ).set({ enable: true } );
        $( this.snap.node ).bind( 'pinchstart', function( ev ) {
            var game = $( this ).data( 'game' );
            game.zoomStart = distance( ev.gesture.pointers );
        }).bind( 'pinchmove', function( ev ) {
            var game = $( this ).data( 'game' );
            var zoom = distance( ev.gesture.pointers ) / game.zoomStart;
            game.panZoom( game.center, zoom );
        }).bind( 'pinchcancel', function( ev ) {
            var game = $( this ).data( 'game' );
            game.panZoom([50,50],1);
        });

        $( this.snap.node ).bind( 'panstart', function( ev ) {
            var game = $( this ).data( 'game' );
            var m = game.snap.select( '#hexes' ).node.getScreenCTM();
            var p = game.snap.node.createSVGPoint();
            p.x = ev.gesture.center.x;
            p.y = ev.gesture.center.y;
            game.panStart = p.matrixTransform(m.inverse());
            game.panStart = [ game.center[ 0 ], game.center[ 1 ] ];
        }).bind( 'panmove', function( ev ) {
            var game = $( this ).data( 'game' );
            var pan = [ 
                game.panStart[ 0 ] - 100 * ev.gesture.deltaX / window.innerWidth,
                game.panStart[ 1 ] - 100 * ev.gesture.deltaY / window.innerHeight,
            ]
            game.panZoom( pan, game.zoomLevel );
        }).bind( 'pancancel', function( ev ) {
            var game = $( this ).data( 'game' );
            game.panZoom([50,50],game.zoomLevel);
        });
    }

    panZoom( pos, zoom ) {
        pos = pos.map( x => Math.min( Math.max( x, 0 ), 100 ) );

        var x = 50 - zoom * pos[ 0 ];
        var y = 50 - zoom * pos[ 1 ];
        var z = zoom;
        this.center = pos;
        this.zoomLevel = zoom;
        this.snap.select( '#hexes' ).attr( 'transform', `matrix(${z},0,0,${z},${x},${y})` );
    }

    initialise( ev ) {
        ev.preventDefault();
        var game = this.data( 'game' );
        game.mines = Snap.set();
        game.flags = Snap.set();
        game.frees = Snap.set();

        game.hexes.forEach( hex => hex.attr( "class", "hex" ) );
        game.hexes.forEach( hex => hex.select( "text" ).node.textContent = "?" );

        game.hexes.forEach(
            hex => hex.mouseup( this.toggleMine )
//                     .touchend( this.toggleMine )
                      .unmouseup( this.initialise )
//                      .untouchend( this.initialise )
        );

        if( game.first ) {
            game.remine( this );
            game.first = false;
            game.toggleMine.call( this, ev );
        }
    }

    rebuild() {
        var width = this.snap.node.width.baseVal.valueInSpecifiedUnits;
        var cx = width / 2;
        var cy = width / 2; 

        this.innerRadius = 0.5 * ( width - 1 ) / ( ( 2 * this.outerRadius + 1 ) * Math.cos( Math.PI / 6 ) );
        this._h = 2 * this.innerRadius * Math.cos( Math.PI / 6 );
        this._j[ 0 ] = this.innerRadius * Math.cos( Math.PI / 6 );
        this._j[ 1 ] = 2 * this._j[ 0 ] * Math.cos( Math.PI / 6 );

        function makeHex() {
            var hexVectors = [];
            for(var i=0; i<6; i+=1) {
                hexVectors.push( 
                    this.innerRadius * Math.sin( i * Math.PI / 3 ), 
                    this.innerRadius * Math.cos( i * Math.PI / 3 )
                );
            }
            var hex = this.hexGroup.g();
            hex.attr( 'class', 'hex' );
            hex.polygon( hexVectors );
            var text = hex.text( -this.innerRadius / 3 , +this.innerRadius/3, '?' );

            text.attr({ 'font-size': this.innerRadius });
            return hex;
        }

        this.hexGroup = this.snap.g({id:"hexes"});
        var prev = this.snap.select( '#hexes' );
        if( prev !== null ) {
            prev.remove()
        }
        var template_hex = makeHex.call( this );

        this.hexes = Snap.set();
        for( var h=-this.outerRadius; h<=this.outerRadius; h=h+1 ) {
            for( var j=-this.outerRadius; j<=this.outerRadius; j=j+1 ) {
                if( Math.abs( h + j ) > this.outerRadius)
                    continue;
                var x = cx + h * this._h + j * this._j[ 0 ];
                var y = cy + j * this._j[ 1 ];

                var hex = template_hex.clone();
                hex.data( 'position', [ h, j ] );
                hex.data( 'cartesian', [ x, y ] );

                var m = hex.transform().localMatrix;
                m.translate( x, y );
                hex.transform( m ); 
                this.hexes.push( hex );
            }
        }
        template_hex.remove();
        this.hexGroup.appendTo( this.snap );
        return this.hexes;
    }

    neighbours( hex ) {
        var neighbour_offsets = [ 
            [  0, +1 ],
            [  0, -1 ],
            [ -1,  0 ],
            [ +1,  0 ],
            [ +1, -1 ],
            [ -1, +1 ],
        ];
        var position = hex.data( 'position' );
        var neighbour_positions = neighbour_offsets.map(
            p => [ position[ 0 ] + p[ 0 ], position[ 1 ] + p[ 1 ] ].join( "x" )
        );
        var neighbours = this.hexes.items.filter(
            x => neighbour_positions.includes( x.data( 'position' ).join( "x" ) )
        );
        return neighbours;
    }

    remine( ignore ) {
        var scheme = getSubScheme( colourScheme, "blank" );
        this.hexes.forEach( h => h.data({ "is mine": false }));
        this.hexes.forEach( h => h.attr( "class", "hex" ) );
        var hexes = this.hexes.items.filter( x => x != ignore );
        this.mines = Snap.set( ...chance.pick( hexes, this.mineRatio * this.hexes.length ) );
        this.mines.forEach( m => m.data( "is mine", true ) );
        this.frees = Snap.set( ...this.hexes.items.filter( x => ! x.data( "is mine" ) ) );  
        this.hexes.forEach(
            hex => hex.mouseup( this.toggleMine )
        )
            
    }

    toggleMine( ev ) {
        var game = this.data( 'game' );
        if( game.mines.length == 0 ) {
            game.remine( this );
            return;
        }
        
        if( this.attr( "class" ) == "hex" ) {
            if( this.data( "is mine" ) ) {
                this.attr( "class", "hex mine" );
                this.select( 'text' ).node.textContent = '*';
                game.endGame();
            } else {
                this.attr( "class", "hex free" )
                var neighbours = game.neighbours( this );
                var mine_neighbours = neighbours.filter(
                    x => x.data( 'is mine' )
                );
                if( mine_neighbours.length == 0 ) {
                    this.select( 'text' ).node.textContent = "";
                    neighbours.map( hx => game.toggleMine.call( hx, ev ) );
                } else {
                    this.select( 'text' ).node.textContent = mine_neighbours.length;
                }
            }
        }
    }

    endGame() {
        function activate() {
            if( this.attr( "class" ) == "hex" ) {
                if( this.data( "is mine" ) ) {
                    this.attr( "class", "hex mine" );
                    this.select( 'text' ).node.textContent = '*';
                } else {
                    this.attr( "class", "hex free" )
                    var neighbours = game.neighbours( this );
                    var mine_neighbours = neighbours.filter(
                        x => x.data( 'is mine' )
                    );
                    if( mine_neighbours.length == 0 ) {
                        this.select( 'text' ).node.textContent = "";
                    } else {
                        this.select( 'text' ).node.textContent = mine_neighbours.length;
                    }
                }
            }
        }
        this.hexes.forEach( hx => activate.call( hx ) );
        this.hexes.forEach(
            hex => hex.unmouseup( this.toggleMine )
                      .mouseup( this.initialise )
        );
    }
}
