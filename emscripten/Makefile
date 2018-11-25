all: build/sweeper.js

build:
	mkdir build

build/sweeper.js: build build/glue.js build/glue.cpp main.cpp src/* include/*
	emcc -s WASM=0 -s MODULARIZE=1 -s EXPORT_NAME=Sweeper -s DEMANGLE_SUPPORT=1\
        --pre-js module.js \
        --post-js build/glue.js \
        --preload-file ../layouts\
        -o build/sweeper.js \
        -L/usr/lib -Iinclude -Ibuild \
        $(shell pkg-config --cflags --libs json-c) \
        src/*.cpp main.cpp

html: build build/glue.js build/glue.cpp main.cpp src/* include/*
	emcc -s WASM=0 -s DEMANGLE_SUPPORT=1\
        --post-js build/glue.js \
        --preload-file ../layouts\
        -o build/sweeper.html \
        --shell-file shell.html \
        -L/usr/lib -Iinclude -Ibuild \
        $(shell pkg-config --cflags --libs json-c) \
        src/*.cpp main.cpp


build/glue.js: idl/*
	cd build; python /usr/lib/emscripten/tools/webidl_binder.py ../idl/game.idl glue

build/glue.cpp: build build/glue.js

clean:
	rm -r build
