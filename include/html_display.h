#include <emscripten.h>
#include <emscripten/html5.h>
#include "game.h"

class HTMLDisplay: public Display {
    public:
        HTMLDisplay( Game* game);
};
