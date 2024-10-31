all: build

build: 
	g++ main.cpp -o main.out

run: build
	./main.out 4096 1 FIFO Global combined_short.trace
	./main.out 4096 1 LRU Global combined_short.trace

run2: build
	./main.out 4096 1 FIFO Global combined.trace
	./main.out 4096 1 LRU Global combined.trace
