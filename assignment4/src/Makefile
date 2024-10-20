./1.out: part1.cpp libppm.cpp
	g++ -g -w part1.cpp libppm.cpp -o a1.out

./2_1.out: part2_1.cpp libppm.cpp
	g++ -g -w part2_1.cpp libppm.cpp -o a2_1.out -lz

./2_2.out: part2_2.cpp libppm.cpp
	g++ -g -w part2_2.cpp libppm.cpp -o a2_2.out

./2_3.out: part2_3.cpp libppm.cpp
	g++ -g -w part2_3.cpp libppm.cpp -o a2_3.out

part1: ./1.out
	./a1.out ../images/1.ppm ../images/output_part1.ppm

part2_1: ./2_1.out
	./a2_1.out ../images/1.ppm ../images/output_part2_1.ppm

part2_3: ./2_3.out
	./a2_3.out ../images/1.ppm ../images/output_part2_3.ppm

part2_2:	./2.out
	./a2_2.out ../images/1.ppm ../images/output_part2_2.ppm

clean:
	rm *.out

.PHONY: part1 part2_1 part2_3 part2_2