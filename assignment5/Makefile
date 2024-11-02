all: build

build: 
	g++ main.cpp -o main.out

run: build
	./main.out 4096 1024 LRU Global combined.trace

run2: build
	./main.out 4096 1024 OPTIMAL Global combined.trace