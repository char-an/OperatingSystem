.PHONY: part1 part2_1

# build-part1: ./a.out

# ./a.out: part1.cpp libppm.cpp
# 	g++ -g -w part1.cpp libppm.cpp

./1.out: part1.cpp libppm.cpp
	g++ -g -w part1.cpp libppm.cpp -o a1.out

./2_1.out: part2_1.cpp libppm.cpp
	g++ -g -w part2_1.cpp libppm.cpp -o a2_1.out

./2_2.out: part2_2s1.cpp part2_2s2.cpp libppm.cpp
	g++ -g -w part2_2s1.cpp libppm.cpp -o a2_2s1.out
	g++ -g -w part2_2s2.cpp libppm.cpp -o a2_2s2.out

# run-sharpen: ./a.out
# 	./a.out ../images/$(INPUT).ppm ../images/$(OUTPUT).ppm

part1: ./1.out
	./a1.out ../images/6.ppm ../images/output_part1.ppm

part2_1: ./2_1.out
	./a2_1.out ../images/6.ppm ../images/output_part2_1.ppm

part2_2: ./2_2.out
	./a2_2s1.out ../images/6.ppm ../images/output_part2_2s1.ppm
	./a2_2s2.out ../images/6.ppm ../images/output_part2_2s2.ppm


clean:
	rm a.out