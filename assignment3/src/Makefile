all: build

build:
	g++ schedular.cpp -o schedular.out

build_two:
	g++ -DTWO schedular.cpp -o schedular.out

run:
	./schedular.out FIFO process1.dat
	./schedular.out FIFO process2.dat
	./schedular.out FIFO process3.dat
	./schedular.out SJF process1.dat
	./schedular.out SJF process2.dat
	./schedular.out SJF process3.dat
	./schedular.out SRTF process1.dat
	./schedular.out SRTF process2.dat
	./schedular.out SRTF process3.dat
	./schedular.out RR process1.dat
	./schedular.out RR process2.dat
	./schedular.out RR process3.dat



	
