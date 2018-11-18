class Game {
    constructor( radius, mineRatio ) {
        var svg = this.svg = document.createElementNS( "http://www.w3.org/2000/svg", "svg" );
        document.body.appendChild( svg );
        svg.setAttribute( "xmlns", "http://www.w3.org/2000/svg" );
        svg.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        svg.setAttribute( "viewBox", "0 0 100 100" );
        svg.setAttribute( "id", "s" );
        svg.setAttribute( "version", "1.1" );

        var style = this.style = document.createElementNS( "http://www.w3.org/2000/svg", "style" );
        svg.appendChild( style );
        style.setAttribute( "id", "style" );

        var reload = this.reload = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
        reload.setAttribute( "id", "reload" );
        reload.setAttribute( "transform", "matrix(0.5,0,0,0.5,88,4)" );
        var back = document.createElementNS( "http://www.w3.org/2000/svg", "rect" );
        back.setAttribute( "class", "back" );
        back.setAttribute( "width", 16 );
        back.setAttribute( "height", 16 );
        reload.appendChild( back );        

        var contents = document.createElementNS( "http://www.w3.org/2000/svg", "path" );
        contents.setAttribute( "d", "M 12.007 8.01 L 12.007 8.488 L 12 8.488 C 12 10.974 9.985 12.988 7.5 12.988 C 5.015 12.988 3 10.974 3 8.488 C 3 6.17 4.759 4.284 7.012 4.037 L 7.012 6.549 L 11.519 3.775 L 7.012 1 L 7.012 3.013 C 4.203 3.26 2 5.615 2 8.488 C 2 11.526 4.462 13.988 7.5 13.988 C 10.509 13.988 12.95 11.571 12.996 8.572 L 13 8.572 L 13 8.01 L 12.007 8.01 Z" );
        
        reload.appendChild( contents );
        svg.appendChild( reload );
        var this_ = this;
        reload.onmouseup = function() {
            this_.init();
        }

        this.count = 1 + 3 * radius * ( radius + 1 );
        this.mineCount = Math.round( this.count * mineRatio );
        this.cells = [];
        var cellRadius = 57.1593533487 / ( 2 * radius + 1 );

        this.setStyle( 
            1.0,  // transition speed
            "#fff", // stroke style
            0.1 * cellRadius, // unknown cell stroke width
            "rgba(0,0,0,0)", // unknown cell fill
            0.2 * cellRadius, // hover stroke width
            "#008900", // free fill
            "#ec0000", // mine fill
            "#b95c00" // flag fill
        );



        //seed = time( NULL );

        //radius = get_outer_radius();
        //if( radius < 0 ) radius = 3;

        var cartesianConverters = [
            1.7320 * cellRadius,
            0.8660 * cellRadius,
            1.5 * cellRadius
        ];

        var count = 0;
        var rows = [];
        for( var i=-radius; i<=radius; i++) {
            for( var j=-radius; j<=radius; j++) {
                if( Math.abs( i + j ) > radius ) continue;
                var x = 50 + j * cartesianConverters[ 0 ] + i * cartesianConverters[ 1 ];
                var y = 50 + i * cartesianConverters[ 2 ];
                this.cells[ count ] = new Cell( this, count, x, y, cellRadius );
                count += 1;
            }
            rows[ i + radius ] = count;
        }
        var row = 0;
        var prev_row_length = -1;
        var row_length = rows[ 0 ];
        var next_row_length = rows[ 1 ] - rows[ 0 ];

        for( var i=0; i<this.count; i++) {
            if( i == rows[ row ] ) {
                row++;
                prev_row_length = row_length;
                row_length = rows[ row ] - rows[ row - 1 ];
                next_row_length = row < 2 * radius + 1 ? rows[ row + 1 ] - rows[ row ] : -1;
            }

            var top_ = rows[ row ] - row_length == i && row >= radius;
            var top_right = row == 2 * radius;
            var bottom_right = rows[ row ] == i + 1 && row >= radius;
            var bottom_ = rows[ row ] == i + 1 && row <= radius;
            var bottom_left = row == 0;
            var top_left = rows[ row ] - row_length == i && row <= radius;

            var neighbour_ids = [
                i + Math.max( row_length, next_row_length ) - 1,
                i + Math.max( row_length, next_row_length ),
                i + 1,
                i - Math.max( row_length, prev_row_length ) + 1,
                i - Math.max( row_length, prev_row_length ),
                i - 1
            ];

            this.cells[ i ].neighbours[ 0 ] = !( top_ || top_right ) ? this.cells[ Math.floor( i + Math.max( row_length, next_row_length ) - 1  ) ]: undefined;
            this.cells[ i ].neighbours[ 1 ] = !( top_right || bottom_right ) ? this.cells[ Math.floor( i + Math.max( row_length, next_row_length )  ) ]: undefined;
            this.cells[ i ].neighbours[ 2 ] = !( bottom_right || bottom_ ) ? this.cells[ Math.floor( i + 1  ) ]: undefined;
            this.cells[ i ].neighbours[ 3 ] = !( bottom_ || bottom_left ) ? this.cells[ Math.floor( i - Math.max( row_length, prev_row_length ) + 1 ) ]: undefined;
            this.cells[ i ].neighbours[ 4 ] = !( bottom_left || top_left ) ? this.cells[ Math.floor( i - Math.max( row_length, prev_row_length ) ) ]: undefined;
            this.cells[ i ].neighbours[ 5 ] = !( top_ || top_left ) ? this.cells[ Math.floor( i - 1 ) ]: undefined;
            this.cells[ i ].neighbours = this.cells[ i ].neighbours.filter( x => x !== undefined );
        }

    }

    init() {
        this.svg.setAttribute( "style", "opacity: 0" );
        this.reload.setAttribute( "class", "");
        this.mines = chance.pickset( this.cells, this.mineCount );
        this.cells.map( x => x.activated = 0 );
        this.cells.map( x => x.setClass( "hex" ) );
        this.cells.map( x => x.mineCount = 0 );
        this.cells.map( x => x.isMine = 0 );
        this.mines.map( x => x.isMine = 1 );
        this.mines.map( x => x.neighbours.map( y => y.mineCount += 1 ) );
        var this_ = this;
        window.setTimeout( function() {
            this_.svg.setAttribute( "style", "" );
        }, 1000 );
    }

    setStyle( transition_speed, stroke, unknown_stroke_width, 
              unknown_stroke_fill, hover_stroke_width, free_fill, mine_fill, 
              flag_fill ) {
        this.style.innerHTML = `
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

        .hex.neighbour > polygon {
            stroke-width: ${hover_stroke_width};
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

        #reload {
            cursor: pointer;
            fill: white;
            display: none;
        }

        #reload .back {
            opacity: 0;
        }

        #reload.win {
            fill: ${free_fill};
            display: initial;
        }

        #reload.lose {
            fill: ${mine_fill};
            display: initial;
        }

    `;
    }

}
