/* for recolouring */

// these two schemes have optimum contrast, but currently don't cater for 
// colour blindness

var darkColourScheme = {
    'background': '#000',
    'default': {
        'normal': { 'stroke': '#fff', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'hover': { 'stroke': '#fff', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'activate': { 'stroke': '#fff', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 }
    },
    'text': {
        'normal': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'hover': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'activate': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 }
    },
    'hidden-text': {
        'normal': { 'stroke': 'rgba(0,0,0,0)', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'hover': { 'stroke': 'rgba(0,0,0,0)', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'activate': { 'stroke': 'rgba(0,0,0,0)', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 }
    },
    'blank': {
        'hover': { 'fill': '#a5a5a5', 'strokeWidth': 1.0 }
    },
    'icon': {
        'normal': { 'fill': '#fff' },
        'hover': { 'fill': '#a5a5a5', 'strokeWidth': 1.0 },
    },
    'flag': {
        'normal': { 'fill': '#b95c00' },
        'hover': { 'fill': '#aaf', 'strokeWidth': 1.0 },
    },
    'free': {
        'normal': { 'fill': '#008900' },
        'hover': { 'fill': '#afa', 'strokeWidth': 1.0 },
    },
    'mine': {
        'normal': { 'fill': '#ec0000' },
        'hover': { 'fill': '#faa', 'strokeWidth': 1.0 },
    },
};

var brightColourScheme = {
    'background': '#fff',
    'default': {
        'normal': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'hover': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'activate': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 }
    },
    'text': {
        'normal': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'hover': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 },
        'activate': { 'stroke': '#000', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.5 }
    },
    'hidden-text': {
        'normal': { 'stroke': 'rgba(0,0,0,0)', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.0 },
        'hover': { 'stroke': 'rgba(0,0,0,0)', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.0 },
        'activate': { 'stroke': 'rgba(0,0,0,0)', 'fill': 'rgba(0,0,0,0)', 'strokeWidth': 0.0 }
    },
    'blank': {
        'hover': { 'fill': '#505050', 'strokeWidth': 1.0 }
    },
    'icon': {
        'normal': { 'fill': '#000' },
        'hover': { 'fill': '#505050', 'strokeWidth': 1.0 },
    },
    'flag': {
        'normal': { 'fill': '#b95c00' },
        'hover': { 'fill': '#ff0', 'strokeWidth': 1.0 },
    },
    'free': {
        'normal': { 'fill': '#008700' },
        'hover': { 'fill': '#5f5', 'strokeWidth': 1.0 },
    },
    'mine': {
        'normal': { 'fill': '#ec0000' },
        'hover': { 'fill': '#f55', 'strokeWidth': 1.0 },
    },
};

function getSubScheme( scheme /*, *priorities*/ ) {
    var priority = Array.prototype.slice.call (arguments, 1);
    priority.push( 'default' );
    var subScheme = { 'normal': {}, 'hover': {}, 'active': {} };
    var keys = Object.keys( subScheme );
    for( var i=0; i<priority.length; i++ ) {
        var p = scheme[ priority[ i ] ]; 
        for( var j=0; j<keys.length; j++ ) {
            var key = keys[ j ];
            if( p[ key ] === undefined )
                continue;
            var jkeys = Object.keys( p[ key ] );
            for( var k=0; k<jkeys.length; k++ ) {
                var jkey = jkeys[ k ];
                if( subScheme[ key ][ jkey ] === undefined ) {
                    subScheme[ key ][ jkey ] = p[ key ][ jkey ];
                }
            }
        }
    }
    return subScheme;
}

var colourScheme = darkColourScheme;

eve.on( 'updateColourScheme', function( scheme ) {
    $( 'body' ).css({
         'background-color': colourScheme.background,
         'transition': 'background-color 1s',
    });
});

function updateColourScheme( scheme ) {
    colourScheme = scheme;
    eve( 'updateColourScheme', 'self', scheme );
}

/* visual focus indication */

// nothing yet, but should enlarge on focus or something

/* motor disabilities */

// nothing yet but there should be multiple ways of interacting and keys
// should be remappable
// don't assume multiple fingers etc
