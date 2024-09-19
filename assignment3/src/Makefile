all: build

# build: schedular.cpp
# 	g++ -O3  -Dfifo schedular.cpp -o fifo_schedular.out
# 	g++ -O3 schedular.cpp -o sjf_schedular.out
    
# run: 
# 	./bin/fifo_schedular.out process1.dat
# 	./bin/fifo_schedular.out process2.dat
# 	./bin/fifo_schedular.out process3.dat
# 	./bin/sjf_schedular.out process1.dat 
# 	./bin/sjf_schedular.out process2.dat
# 	./bin/sjf_schedular.out process3.dat

build:
	g++ -O3 -w schedular.cpp -o schedular

build_two:
	g++ -O3 -w -DTWO schedular.cpp -o schedular

run:
	./schedular FIFO process1.dat