class SimpleHexSweeper {
    constructor( snap, outerRadius, mineRatio ) {
        this.snap = snap;
        this.mineRatio = mineRatio;
        this.outerRadius = outerRadius;

        this.innerRadius = 0;
        this.hexes = [];
        this.mines = [];
        this.flags = [];
        this.frees = [];

        this._j = [];
        this._h = 0;

        this.selectAction = undefined;

        this.rebuild();
        this.remine();
        eve( "updateColourScheme", "self", colourScheme );
    }

    rebuild() {
        var width = this.snap.node.width.baseVal.value;
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
            hex.polygon( hexVectors );
            return hex;
        }
        function defaultMouseover() { 
            this.animate( this.data( "scheme" ).hover, 300, mina.easein );
        }
        function defaultMouseout() {
            this.animate( this.data( "scheme" ).normal, 300, mina.easeout );
        }

        this.hexGroup = this.snap.g({id:"hexes"});
        var prev = this.snap.select( '#hexes' );
        if( prev !== null ) {
            prev.remove()
        }
        var template_hex = makeHex.apply( this );

        this.hexes = [];
        var scheme = getSubScheme( colourScheme, "blank" );
        for( var h=-this.outerRadius; h<=this.outerRadius; h=h+1 ) {
            var H = Math.abs( h );
            for( var j=-this.outerRadius; j<=this.outerRadius; j=j+1 ) {
                var J = Math.abs( j );
                if( Math.abs( h + j ) > this.outerRadius)
                    continue;
                var x = cx + h * this._h + j * this._j[ 0 ];
                var y = cy + j * this._j[ 1 ];
                var hex = template_hex.clone();
                var m = hex.transform().localMatrix;
                m.translate( x, y );
                hex.transform( m ); 
                this.hexes.push( hex );

                hex.data( "state", "blank" );
                hex.data( "scheme", scheme );
                hex.attr( scheme.normal);
                hex.mouseover( defaultMouseover )
                   .mouseout( defaultMouseout )
                   .mouseup( this.toggleMine )
                   .touchstart( defaultMouseover )
                   .touchcancel( defaultMouseout )
                   .touchend( function(){ 
                        toggleMine.call( this );
                        defaultMouseout.call( this );
                    });

                var updateColour = function( h ) { 
                    return function( scheme ) {
                        h.data( "scheme", getSubScheme( scheme, "blank" ) );
                        h.animate( h.data( "scheme" ).normal, 300 );
                    }
                }
                eve.on( "updateColourScheme", updateColour( hex ) );
            }
        }
        template_hex.remove();
        this.hexGroup.appendTo( this.snap );
        return this.hexes;
    }

    remine() {
        var scheme = getSubScheme( colourScheme, "blank" );
        this.hexes.map( h => h.data({
            "is mine": false,
            "state": "blank",
            "scheme": scheme
        }));
        this.mines = chance.pick( this.hexes, this.mineRatio * this.hexes.length );
        this.mines.map( m => m.data( "is mine", true ) );
        this.frees = this.hexes.filter( x => ! x.data( "is mine" ) ); 
        eve( "updateColourScheme", "self", colourScheme );
    }

    toggleMine( element ) {
        if( this.data( "state" ) == "blank" ) {
            if( this.data( "is mine" ) ) {
                this.data( "scheme", getSubScheme( colourScheme, "mine" ) );
                this.data( "state", "mine" )
            } else {
                this.data( "scheme", getSubScheme( colourScheme, "free" ) );
                this.data( "state", "free" )
            }
        } 
        this.animate( this.data( "scheme" ).hover, 100 );
    }
}
