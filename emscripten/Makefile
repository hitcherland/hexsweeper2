output/html/index.html: html/index.html src/game.cpp src/html.cpp js/* css/*
	@mkdir -p output/html ||:
	@cp -r js/ css/ output/html ||:
	emcc -I include -s EXPORTED_FUNCTIONS='["_update_radius","_main"]' -std=c++11 --pre-js js/pre.js --shell-file html/index.html -o output/html/index.html src/html.cpp src/game.cpp 


all: output/js/main.js output/html/index.html

output/js/main.js: src/main.cpp
	@mkdir -p output/js ||:
	emcc -o output/js/main.js src/main.cpp 

test_js: output/js/main.js
	node output/js/main.js

clean:
	rm -rf output
