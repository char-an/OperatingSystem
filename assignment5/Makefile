all: build

build: 
	g++ -w main.cpp -o main

run: build
	./main 4096 10 FIFO Global combined_short.trace

test: build
	./main 4096 1 FIFO Global combined_short.trace