all: build

build: 
	g++ main.cpp -o main.out

run: build
	# ./main.out 4096 100 FIFO Local combined.trace
	# ./main.out 4096 100 LRU Local combined.trace
	./main.out 4096 100 RANDOM Local combined.trace

run2: build
	# ./main.out 4096 100 FIFO Global combined.trace
	# ./main.out 4096 100 LRU Global combined.trace
	./main.out 4096 100 RANDOM Global combined.trace