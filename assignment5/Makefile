all: build

build: 
	g++ -O3 main.cpp -o main.out

run: build
	./main.out 4096 8 LRU Global combined.trace

run2: build
	./main.out 4096 512 OPTIMAL Local combined.trace