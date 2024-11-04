all: build

build: 
	g++ main.cpp -o main.out

run: build
	./main.out 4096 100 FIFO Local combined.trace

run2: build
	./main.out 4096 100 FIFO Global combined.trace