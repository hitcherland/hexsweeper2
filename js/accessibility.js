/* for recolouring */

// these two schemes have optimum contrast, but currently don't cater for 
// colour blindness

var defaultColourScheme = {
    'background': '#000',
    'stroke': '#fff',
    'blankFill': '#000',
    'flagFill': '#77f',
    'freeFill': '#7f7',
    'mineFill': '#f77',
};

var invertColourScheme = {
    'background': '#fff',
    'stroke': '#000',
    'blankFill': '#fff',
    'flagFill': '#00f',
    'freeFill': '#0f0',
    'mineFill': '#f00',
};

var colourScheme = defaultColourScheme;

eve.on( 'updateColourScheme', function( scheme ) {
    $( 'body' ).css( 'background-color', scheme.background);
});

function updateColourScheme( scheme ) {
    eve( 'updateColourScheme', 'self', scheme );
}

/* visual focus indication */

// nothing yet, but should enlarge on focus or something

/* motor disabilities */

// nothing yet but there should be multiple ways of interacting and keys
// should be remappable
// don't assume multiple fingers etc
