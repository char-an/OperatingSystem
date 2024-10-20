build-sharpen: ./a.out

./a.out: image_sharpener.cpp libppm.cpp
	g++ -g image_sharpener.cpp libppm.cpp

run-sharpen: ./a.out
	./a.out ../images/1.ppm ../images/1_out.ppm

clean:
	rm a.out
