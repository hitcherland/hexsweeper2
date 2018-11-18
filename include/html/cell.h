#ifndef __sweeper_html_cell__
#define __sweeper_html_cell__

#include <emscripten.h>
#include <emscripten/html5.h>
#include <cell.h>

class HTMLCell: public Cell {
    public:
        HTMLCell();
        HTMLCell( int id, float x, float y, float radius );
        void init( int id, float x, float y, float radius );
        void activate();
        void addClass( const char* classname);
        void removeClass( const char* classname);
        void showMineCount();

        HTMLCell* neighbours[ 6 ];
};

#endif
