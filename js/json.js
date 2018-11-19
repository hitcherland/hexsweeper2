var TemplateEngine = function(html, options) {
    var re = /\${([^}]+)?}/g, reExp = /(^( )?(if|for|else|switch|case|break|{|}))(.*)?/g, code = 'var r=[];\n', cursor = 0, match;
    var add = function(line, js) {
        js? (code += line.match(reExp) ? line + '\n' : 'r.push(' + line + ');\n') :
            (code += line != '' ? 'r.push("' + line.replace(/"/g, '\\"') + '");\n' : '');
        return add;
    }
    while(match = re.exec(html)) {
        add(html.slice(cursor, match.index))(match[1], true);
        cursor = match.index + match[0].length;
    }
    add(html.substr(cursor, html.length - cursor));
    code += 'return r.join("");';
    return new Function(code.replace(/[\r\t\n]/g, '')).apply(options);
}

function getGameData( path, callback ) {
    var oReq = new XMLHttpRequest();
    var data;
    oReq.onload = function( e ) {
        data = JSON.parse( this.responseText );
        callback( data );
    }
    oReq.open( "get", path );
    oReq.send();
    return data;
}

function makeGame( json ) {
    function template(strings, ...keys) {
      return (function(...values) {
        var dict = values[values.length - 1] || {};
        var result = [strings[0]];
        keys.forEach(function(key, i) {
          var value = Number.isInteger(key) ? values[key] : dict[key];
          result.push(value, strings[i + 1]);
        });
        return result.join('');
      });
    }

    var bodyHTML = TemplateEngine( json.body, json.variables );
    
    document.body.innerHTML = bodyHTML;
    var svg = document.body.lastElementChild;
    
    var cellTemplates = {};
    var cellNames = Object.keys( json.cells );
    for( var i=0; i<cellNames.length; i++) {
        var cellName = cellNames[ i ];
        var cell = json.cells[ cellName ];
        cellTemplates[ cellName ] = cell.html
    }

    var cells = [];
    var __keys__ = [];

    function addCell( type, units ) {
        var unit_str = Object.values( units ).join( ":" );
        var __key__ = `${type}:${unit_str}`
        if( __keys__.indexOf( __key__ ) >= 0 )
            return;
        __keys__.push( __key__ );
 
        var attributes = { "data:type": type };       

        var position = [ 0, 0 ];
        var unitNames = Object.keys( units );
        for( var i=0; i<unitNames.length; i++) {
            var unitName = unitNames[ i ];
            var unit = units[ unitName ];
            attributes[ "data:unit." + unitName.toLowerCase() ] = unit;
            var offset = json.units[ unitName ].map( x => x * unit );
            position[ 0 ] += offset[ 0 ];
            position[ 1 ] += offset[ 1 ];
        }

        var html = TemplateEngine( cellTemplates[ cellType ], { x: position[ 0 ], y: position[ 1 ] } );
        svg.innerHTML += html;
        var cell = svg.lastElementChild;

        var attributeNames = Object.keys( attributes );
        for( var i=0; i<attributeNames.length; i++) {
            var attributeName = attributeNames[ i ];
            var attributeValue = attributes[ attributeName ];
            cell.setAttribute( attributeName, attributeValue );
        }
        cells.push( cell );
        return cell;
    }

    var seed = json.layout.seed;
    var unhandled = [];
    for( var i=0; i<seed.length; i++) {
        var cellType = seed[ i ].cell;
        var position = seed[ i ].position;
        var cell = addCell( cellType, position );
        unhandled.push( cell );
    }
    var addedSomething = true;
    var count = 0;

    var rule = json.layout.rule; //( json.variables, 

    while( addedSomething ) {
        addedSomething = false;
        var new_unhandled = [];
        for( var i=0; i<unhandled.length; i++ ) {
            var cell = unhandled[ i ];
            var cellType = cell.getAttribute( "data:type" );

            var units = {};
            var unitNames = Object.keys( json.units );
            for( var j=0; j<unitNames.length; j++) {
                var unitName = unitNames[ j ];
                units[ unitName ] = parseInt( cell.getAttribute( "data:unit." + unitName.toLowerCase() ) || 0 );
            }

            var neighbours = json.cells[ cellType ].neighbours;
            for( var j=0; j<neighbours.length; j++) {
                var neighbourType = neighbours[ j ].type;
                var neighbourPosition = neighbours[ j ].position;

                var neighbourUnits = {}
                for( var k=0; k<unitNames.length; k++) {
                    var unitName = unitNames[ k ];
                    neighbourUnits[ unitName ] = units[ unitName ] + ( neighbourPosition[ unitName ] || 0 );
                }
                var rule_eval = TemplateEngine( rule, { ...json.variables, ...neighbourUnits } );
                var allowed = eval( rule_eval );
                if( allowed ) {
                    var neighbour = addCell( neighbourType, neighbourUnits );
                    if( neighbour !== undefined ) {                 
                        new_unhandled.push( neighbour );
                        addedSomething = true;
                    }
                }
            }
        }
        unhandled = new_unhandled;
    }
}

//var basic_game = getGameData( "games/hexagonal_basic.json", makeGame );
//var basic_game = getGameData( "games/square_basic.json", makeGame );
var basic_game = getGameData( "games/triangle_basic.json", makeGame );
