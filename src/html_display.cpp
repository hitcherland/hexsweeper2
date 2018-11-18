#include <emscripten.h>
#include <emscripten/html5.h>
#include <json.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <emscripten.h>
#include <emscripten/html5.h>
#include "html_display.h"

EM_JS(void, make_svg, (), {
  var svg = document.createElementNS( "http://www.w3.org/2000/svg", "svg" );
  document.body.appendChild( svg );
  svg.setAttribute( "xmlns", "http://www.w3.org/2000/svg" );
  svg.setAttribute( "viewBox", "0 0 100 100" );
  svg.setAttribute( "id", "s" );
  svg.setAttribute( "version", "1.1" );

  var style = document.createElementNS( "http://www.w3.org/2000/svg", "style" );
  svg.appendChild( style );
  style.setAttribute( "id", "style" );
});

EM_JS(void, make_reload, (), {
  var reload = document.createElementNS( "http://www.w3.org/2000/svg", "path" );
  reload.setAttribute( "d", "M 12.007 8.01 L 12.007 8.488 L 12 8.488 C 12 10.974 9.985 12.988 7.5 12.988 C 5.015 12.988 3 10.974 3 8.488 C 3 6.17 4.759 4.284 7.012 4.037 L 7.012 6.549 L 11.519 3.775 L 7.012 1 L 7.012 3.013 C 4.203 3.26 2 5.615 2 8.488 C 2 11.526 4.462 13.988 7.5 13.988 C 10.509 13.988 12.95 11.571 12.996 8.572 L 13 8.572 L 13 8.01 L 12.007 8.01 Z");
  reload.setAttribute( "id", "reload" );
  reload.setAttribute( "transform", "matrix(1,0,0,1,86,0)" );
  document.getElementById( "s" ).appendChild( reload );
});

HTMLDisplay::HTMLDisplay() {
    //emscripten_set_mouseup_callback( "reload", NULL, EM_TRUE, reload ); 
    make_svg();
    make_reload();
}
