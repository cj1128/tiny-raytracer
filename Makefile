FLAGS := -Wall -Werror -Wconversion -Wno-unused-variable -Wno-missing-braces src/main.cpp -Wno-c++11-compat-deprecated-writable-strings -lSDL2

main.out: src/main.cpp src/tiny_math.h src/tiny_platform.h src/tiny_sdl.h Makefile
	clang++ $(FLAGS) -o main.out

run: main.out
	./main.out

