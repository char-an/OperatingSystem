.PHONY: part1

build-part1: ./a.out

./a.out: part1.cpp libppm.cpp
	g++ -g -w part1.cpp libppm.cpp

run-sharpen: ./a.out
	./a.out ../images/$(INPUT).ppm ../images/$(OUTPUT).ppm

part1: ./a.out
	./a.out ../images/1.ppm ../images/output_part1.ppm

clean:
	rm a.out