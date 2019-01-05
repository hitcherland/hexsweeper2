class Game {
    loadFile( path ) {
        return new Promise( function( resolve, reject ) {
            var oReq = new XMLHttpRequest();
            oReq.open( "get", path );
            oReq.onload = function( e ) {
                resolve( this.responseText );
            }
            oReq.onerror = function( e ) {
                reject( this.responseText );
            }
            oReq.send();
        });
    }

    updateMineRatio( mineRatio ) {
        this.mineRatio = mineRatio;
        this.mineCount = Math.round( this.count * mineRatio );
        this.init();
    }

    updateRadius( radius ) {
        this.radius = radius;

        this.count = 1 + 3 * radius * ( radius + 1 );
        this.mineCount = Math.round( this.count * this.mineRatio );
        this.radius = radius;

        if( this.cells != undefined ) {
            for( var i=0; i<this.cells.length; i++) {
                //console.log( i, this.cells[ i ] );
                this.cells[ i ].domElement.remove();
            }
        }
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
        this.init();
    }


    constructor( radius, mineRatio ) {
        var svg = this.svg = document.createElementNS( "http://www.w3.org/2000/svg", "svg" );
        document.body.appendChild( svg );
        svg.setAttribute( "xmlns", "http://www.w3.org/2000/svg" );
        svg.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
        svg.setAttribute( "viewBox", "0 0 100 100" );
        svg.setAttribute( "id", "s" );
        svg.setAttribute( "version", "1.1" );
        svg.setAttribute( "baseProfile", "tiny" );

        var style = this.style = document.createElementNS( "http://www.w3.org/2000/svg", "style" );
        svg.appendChild( style );
        style.setAttribute( "id", "style" );

        this.makeReload();
        this.makeSettings();

        this.mineRatio = mineRatio;
        this.updateRadius( radius );
    }

    init() {
        this.svg.setAttribute( "style", "opacity: 0" );
        if( this.reload == "undefined" ) {
            this.reload.setAttribute( "class", "");
            
        }
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
        }, 500 );
    }

    makeReload() {
        var this_ = this;
        this.loadFile( "img/reload.svg" ).then( function( content ) {
            var reload = this_.reload = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
            reload.setAttribute( "id", "reload" );
            reload.setAttribute( "transform", "matrix(0.5,0,0,0.5,88,4)" );

            this_.svg.appendChild( reload );

            this_.reload.innerHTML += content;
            var reload = this_.reload = this_.svg.lastElementChild;
            reload.onmouseup = function( e ) {
                if( e.button == 0 ) 
                    this_.init();
            }

            var back = document.createElementNS( "http://www.w3.org/2000/svg", "rect" );
            back.setAttribute( "class", "back" );
            back.setAttribute( "x", 0 );
            back.setAttribute( "y", 0 );
            back.setAttribute( "width", 16 );
            back.setAttribute( "height", 16 );
            reload.firstElementChild.insertBefore( back, reload.firstElementChild.firstElementChild );
        } );
    }

    makeSettings() {
        var this_ = this;
        this.loadFile( "img/settings.svg" ).then( function( content ) {
            var settingsButton = this_.settingsButton = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
            settingsButton.setAttribute( "id", "settingsButton" );
            settingsButton.setAttribute( "transform", "matrix(0.5,0,0,0.5,4,4)" );
            settingsButton.setAttribute( "class", "win" );
            this_.svg.appendChild( settingsButton );

            this_.settingsButton.innerHTML += content;
            var settingsButton = this_.settingsButton = this_.svg.lastElementChild;
            settingsButton.onmouseup = function( e ) {
                if( e.button == 0 ) 
                    this_.toggleSettings();
            }

            var back = document.createElementNS( "http://www.w3.org/2000/svg", "rect" );
            back.setAttribute( "class", "back" );
            back.setAttribute( "x", 0 );
            back.setAttribute( "y", 0 );
            back.setAttribute( "width", 16 );
            back.setAttribute( "height", 16 );
            settingsButton.firstElementChild.insertBefore( back, settingsButton.firstElementChild.firstElementChild );

            var settings = this_.settingsPage = document.createElement( "div" );
            document.body.appendChild( settings );
            settings.setAttribute( "id", "settings" );
            settings.setAttribute( "class", "invisible" );

            var radiusInput = document.createElement( "input" )
            radiusInput.setAttribute( "type", "number" )
            radiusInput.setAttribute( "value", this_.radius );
            radiusInput.setAttribute( "step", 1 );
            radiusInput.setAttribute( "min", 0 );
            radiusInput.onchange = function( e ) { 
                this_.updateRadius( parseInt( this.value ) );
            };
            var radiusInput = this_.makeSettingInput( "radius", {
                "type": "number",
                "value": this_.radius,
                "step": 1,
                "min": 0
            }, function( e ) { 
                this_.updateRadius( parseInt( this.value ) );
            });
            settings.appendChild( radiusInput );

            var mineRatioInput = this_.makeSettingInput( "mine:cell ratio", {
                "type": "range",
                "min": 0,
                "max": 1,
                "step": 0.001,
                "value": this_.mineRatio,
            }, function( e ) { 
                this_.updateMineRatio( parseFloat( this.value ) );
            });
            settings.appendChild( mineRatioInput );

            var okay =document.createElement( "div" );
            okay.setAttribute( "class", "okay" );
            settings.appendChild( okay );
            okay.innerHTML = "BACK";
            okay.onclick = function(e){ this_.toggleSettings(); }

        } );        
    }

    makeSettingInput( name, args, onchange ) {
        var input_div = document.createElement( "div" )
        var label = document.createElement( "label" )
        label.innerHTML = name;
        input_div.appendChild( label );
        var input = document.createElement( "input" )
        input_div.appendChild( input );
        var keys = Object.keys( args );
        for( var i=0; i<keys.length; i++) {
            var key = keys[ i ];
            var value = args[ key ];
            input.setAttribute( key, value );
        }
        input.onchange = onchange;
        return input_div;
    }

    toggleSettings() {
        var state = this.settingsPage.getAttribute( "class" );
        if( state == "invisible" ) {
            this.settingsPage.setAttribute( "class", "visible" );
        } else {
            this.settingsPage.setAttribute( "class", "invisible" );
        }
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

        #settingsButton, #reload { cursor: pointer; }
        #reload path, #settingsButton path { fill: #fff; opacity: 0.4 }
        #settingsButton:hover path, #reload:hover path { opacity: 1; }

        #reload.win path { fill: ${free_fill}; }
        #reload.lose path { fill: ${mine_fill}; }


        #settings {
            position: absolute;
            left: 0;
            top: 0;
            width: 100%;
            min-height: 100%;
            background: black;
            display: initial;
        }

        #settings.invisible {
            display: none;
        }

        #settings > div {
            background: white;
            color: black;
            margin-left: 50%;
            transform:translate( -50% );
            text-align:center;
            font-size: 16px;
            margin-top: 2px;
        }

        #settings .okay {
            color: black;
            background: white;
            min-width: 25%;
            cursor: pointer;
            
        }
    `;
    }

}
