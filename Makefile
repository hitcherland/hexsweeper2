no_wasm:
	/usr/lib/emscripten/emcc -s WASM=0 -o sweeper.js -L/usr/lib -I include/ $(shell pkg-config --cflags --libs json-c) src/game.cpp src/html.cpp --preload-file styles

no_wasm_2:
	/usr/lib/emscripten/emcc -s WASM=0 -o sweeper.js -L/usr/lib -I include/ $(shell pkg-config --cflags --libs json-c) src/game.cpp src/main.cpp --preload-file styles
wasm:
	/usr/lib/emscripten/emcc -o js/sweeper.js src/main.cpp 

html:
	emcc -o sweeper.html src/main.cpp 


