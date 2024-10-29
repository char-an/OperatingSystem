all: build

build: 
	g++ -w main.cpp -o main

run: build
	./main combined_short.trace

exe: build
	./main 4096 10 FIFO Global combined_short.trace