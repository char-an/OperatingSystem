./1.out: part1/part1.cpp part1/libppm.cpp
	g++ -g -w part1/part1.cpp part1/libppm.cpp -o part1/a1.out

./2_1.out: part2_1/part2_1.cpp part2_1/libppm.cpp
	g++ -g -w part2_1/part2_1.cpp part2_1/libppm.cpp -o part2_1/a2_1.out -lz

./2_2.out: part2_2/part2_2.cpp part2_2/libppm.cpp
	g++ -g -w part2_2/part2_2.cpp part2_2/libppm.cpp -o part2_2/a2_2.out

./2_3.out: part2_3/part2_3.cpp part2_3/libppm.cpp
	g++ -g -w part2_3/part2_3.cpp part2_3/libppm.cpp -o part2_3/a2_3.out

part1: ./1.out
	./part1/a1.out 1.ppm output_part1.ppm

part2_1: ./2_1.out
	./part2_1/a2_1.out 1.ppm output_part2_1.ppm

part2_2: ./2_2.out
	./part2_2/a2_2.out 1.ppm output_part2_2.ppm

part2_3: ./2_3.out
	./part2_3/a2_3.out 1.ppm output_part2_3.ppm

clean:
	rm  part1/*.out part2_1/*.out part2_2/*.out part2_3/*.out

.PHONY: part1 part2_1 part2_2 part2_3 clean
