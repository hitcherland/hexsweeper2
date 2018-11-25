#include <game.h>
#include <glue.cpp>

#include <emscripten.h>

EM_JS( void, init, (), {
    layout = new Layout( "hexagonal_basic" );
});

int main() {
    printf( "hello\n" );
    init();
    return 0;
}
