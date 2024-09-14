all: build

build: src/schedular.cpp
	g++ -O3 src/schedular.cpp -o bin/fifo_schedular.out
	g++ -O3 src/schedular.cpp -o bin/sjf_schedular.out
    
run: 
	./bin/fifo_schedular.out process1.dat -DFIFO
	./bin/fifo_schedular.out process2.dat -DFIFO
	./bin/fifo_schedular.out process3.dat -DFIFO
	./bin/sjf_schedular.out process1.dat 
	./bin/sjf_schedular.out process2.dat
	./bin/sjf_schedular.out process3.dat


