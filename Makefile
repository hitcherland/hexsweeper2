all:
	emcc -o js/sweeper.js src/main.cpp 

html:
	emcc -o sweeper.html src/main.cpp 
