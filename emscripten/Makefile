all: output/js/main.js output/html/index.html

output/js/main.js: src/main.cpp
	@mkdir -p output/js ||:
	emcc -o output/js/main.js src/main.cpp 

test_js: output/js/main.js
	node output/js/main.js

output/html/index.html: html/index.html src/main.cpp js/* css/* img/*
	@mkdir -p output/html ||:
	@cp -r js/ css/ img/ output/html ||:
	emcc --pre-js js/pre.js --shell-file html/index.html -o output/html/index.html src/main.cpp 

clean:
	rm -rf output
