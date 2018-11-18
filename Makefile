no_wasm_html:
	/usr/lib/emscripten/emcc -s WASM=0 -I include -L/usr/lib -o sweeper.js $(shell pkg-config --cflags --libs json-c) src/*.cpp src/html/*.cpp --preload-file styles

no_wasm:
	/usr/lib/emscripten/emcc -s WASM=0 -I include -L/usr/lib -o sweeper.js $(shell pkg-config --cflags --libs json-c) src/main.cpp --preload-file styles
wasm:
	/usr/lib/emscripten/emcc -o js/sweeper.js src/main.cpp 

html:
	emcc -o sweeper.html src/main.cpp 


