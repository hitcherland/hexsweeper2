no_wasm:
	/usr/lib/emscripten/emcc -s WASM=0 -o js/sweeper.js src/main.cpp 
wasm:
	/usr/lib/emscripten/emcc -o js/sweeper.js src/main.cpp 

html:
	emcc -o sweeper.html src/main.cpp 


