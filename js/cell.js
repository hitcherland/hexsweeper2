class Cell {
    constructor( game, index, x, y, radius ) {
        var hexPoints = [
            0.0, 1,
            0.866, 0.5,
            0.866, -0.5,
            0, -1,
            -0.866, -0.5,
            -0.866, 0.5,
            0.0, 1
        ];

        this.game = game;
        this.id = index;
        this.isMine = 0;
        this.mineCount = 0;
        this.activated = 0;
        this.neighbours = new Array( 6 ).map( x => null );

        var group = document.createElementNS( "http://www.w3.org/2000/svg", "g" );
        group.id = "h" + this.id;
        group.setAttribute( "transform", "matrix(1,0,0,1," + x + "," + y + ")" );
        group.setAttribute( "class", "hex" );
        document.getElementById( "s" ).appendChild( group );

        var polygon = document.createElementNS( "http://www.w3.org/2000/svg", "polygon" );
        polygon.setAttribute( "points", hexPoints.map( x => x * radius ).join( "," ) );
        group.appendChild( polygon);

        var text = document.createElementNS( "http://www.w3.org/2000/svg", "text" );
        text.id = "t" + this.id;
        text.textContent = "?";
        text.setAttribute( "font-size", radius );
        text.setAttribute( "x", -radius / 3 );
        text.setAttribute( "y",  radius / 3 );

        group.appendChild( text);

        this.domElement = group;
        this.polygon = polygon;
        this.text = text;

        var this_ = this;
        this.domElement.onmouseup = function( ev ) {
            if( ev.button == 0 ) {
                this_.activate();
                simpleSolve( this_.game );
                var result = simpleChecker( this_.game );
                if( result !== undefined ) {
                    alert( result );
                }
            }
        }
    }

    setClass( className ) {
        this.domElement.setAttribute( "class", className );
    }

    addClass( className ) {
        var cell = this.domElement;
        var svg = cell.parentNode;
        var old_class = cell.getAttribute( "class" );
        cell.setAttribute( "class", old_class + " " + className );
        svg.appendChild( cell );
    }

    removeClass( className ) {
        var cell = this.domElement;
        var svg = cell.parentNode;
        var old_class = cell.getAttribute( "class" );
        cell.setAttribute( "class", old_class.replace( className, "" ) );
        svg.appendChild( cell );
    }

    showMineCount() {
        this.text.textContent = this.mineCount == 0 ? "" : this.mineCount; 
    }

    activate() {
        if( this.activated > 0 ) return;

        if( this.isMine == 0 ) {
            this.showMineCount();
            this.addClass( "free" );
        } else {
            this.addClass( "mine" );
        }
        this.activated = 1;
    }
}
