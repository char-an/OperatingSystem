PAGE_SIZE = 4096
FRAME_COUNT = 512

.PHONY: clean

build: main.cpp
	g++ -O3 -w main.cpp -o main.out

run-fifo-global: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) FIFO Global combined.trace
run-fifo-local: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) FIFO Local combined.trace

run-lru-global: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) LRU Global combined.trace
run-lru-local: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) LRU Local combined.trace

run-random-global: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) RANDOM Global combined.trace
run-random-local: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) RANDOM Local combined.trace

run-optimal-global: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) OPTIMAL Global combined.trace
run-optimal-local: build
	./main.out $(PAGE_SIZE) $(FRAME_COUNT) OPTIMAL Local combined.trace

clean:
	rm *.out

