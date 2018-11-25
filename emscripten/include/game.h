#ifndef __sweeper_game_h__
#define __sweeper_game_h__

#include <cstdio>
#include <cstring>
#include <json.h>

class Layout {
    public:
        Layout( const char* sourceFile );
    private:
        int cells;
        const char* source;
        struct json_object* json;
};

#endif
