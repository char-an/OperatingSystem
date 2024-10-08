./1.out: part1.cpp libppm.cpp
	g++ -g -w part1.cpp libppm.cpp -o a1.out

./2_1.out: part2_1.cpp libppm.cpp
	g++ -g -w part2_1.cpp libppm.cpp -o a2.out


part1: ./1.out
	./a1.out ../images/1.ppm ../images/output_part1.ppm

part2_1: ./2_1.out
	./a2.out ../images/1.ppm ../images/output_part1.ppm

clean:
	rm a.out

.PHONY: part1 part2_1 clean