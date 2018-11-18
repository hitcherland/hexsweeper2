

function simpleSolve( game ) {
    var doContinue = true;
    while( doContinue ) {
        doContinue = false;
        var knowns = game.cells.filter( x => x.activated > 0 && x.neighbours.filter( y => y.activated == 0 ).length > 0 )
        var knownLength = knowns.length;
        for( var i=0; i<knownLength; i++) {
            var cell = knowns[ i ];
            var neighbours = cell.neighbours.filter( x => x.activated == 0 );
            if( neighbours.length > 0 && cell.mineCount == 0 ) {
                neighbours.map( x => x.activate() );
                doContinue = true;
            }
        }
    }
}

function simpleChecker( game ) {
    var activatedMines = game.cells.filter( x => x.activated > 0 && x.isMine == 1 )
    if( activatedMines.length > 0 ) {
        return "LOSE";
    }
    var mineCount = game.mines.length;
    var remaining = game.cells.filter( x => x.activated == 0 && x.isMine == 0 )
    if( remaining.length == 0 ) {
        return "WIN";
    }
    return undefined;
}
