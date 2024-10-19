#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <atomic>
#include <thread>

using namespace std;

// atomic flages for synchronization
atomic<bool> Is_s1_done(false);
atomic<bool> Is_s2_done(false);

struct image_t *S1_smoothen(struct image_t *input_image)
{
	// error handling for nullptr
	if (input_image == nullptr)
	{
		cout << "input image in smoothen function is null" << endl;
		return nullptr;
	}

	// initialization
	struct image_t *image = new struct image_t;
	image->height = input_image->height;
	image->width = input_image->width;
	image->image_pixels = new uint8_t **[image->height];
	for (int i = 0; i < image->height; i++)
	{
		image->image_pixels[i] = new uint8_t *[image->width];
		for (int j = 0; j < image->width; j++)
			image->image_pixels[i][j] = new uint8_t[3];
	}

	// logic
	int delrow[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
	int delcol[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};

	for (int i=0;i<input_image->height;i++)
	{
		for (int j=0;j<input_image->width;j++)
		{
			if (i==0 || j==0 || i==input_image->height - 1 || j ==input_image->width - 1)
			{
				image->image_pixels[i][j][0] = input_image->image_pixels[i][j][0];
				image->image_pixels[i][j][1] = input_image->image_pixels[i][j][1];
				image->image_pixels[i][j][2] = input_image->image_pixels[i][j][2];
				continue;
			}
			// for red
			for (int k = 0; k < 9; k++)
			{
				image->image_pixels[i][j][0] += uint8_t(1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][0]));
			}
			// for green
			for (int k = 0; k < 9; k++)
			{
				image->image_pixels[i][j][1] += uint8_t(1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][1]));
			}
			// for blue
			for (int k = 0; k < 9; k++)
			{
				image->image_pixels[i][j][2] += uint8_t(1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][2]));
			}
		}
	}

	return image;
}

struct image_t *S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
	// error handling for nullptr
	if (input_image == nullptr)
	{
		cout << "input image in find_details function is null" << endl;
		return nullptr;
	}
	//|input - smoothened|
	// initialization
	struct image_t *image = new struct image_t;
	image->height = input_image->height;
	image->width = input_image->width;
	image->image_pixels = new uint8_t **[image->height];
	for (int i = 0; i < image->height; i++)
	{
		image->image_pixels[i] = new uint8_t *[image->width];
		for (int j = 0; j < image->width; j++)
			image->image_pixels[i][j] = new uint8_t[3];
	}

	// logic
	for (int i=0;i<input_image->height;i++)
	{
		for (int j=0; j <input_image->width;j++)
		{
			for (int c=0; c <3;c++)
			{
				image->image_pixels[i][j][c] = abs(input_image->image_pixels[i][j][c] - smoothened_image->image_pixels[i][j][c]);
			}
		}
	}

	return image;
}

struct image_t *S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
	// error handling for nullptr
	if (input_image == nullptr)
	{
		cout << "input image in sharpen function is null" << endl;
		return nullptr;
	}
	// input + details
	// initialization
	struct image_t *image = new struct image_t;
	image->height = input_image->height;
	image->width = input_image->width;
	image->image_pixels = new uint8_t **[image->height];
	for (int i = 0; i < image->height; i++)
	{
		image->image_pixels[i] = new uint8_t *[image->width];
		for (int j = 0; j < image->width; j++)
			image->image_pixels[i][j] = new uint8_t[3];
	}

	// logic
	for (int i =0; i <input_image->height; i++)
	{
		for (int j =0; j <input_image->width; j++)
		{
			for (int c =0; c<3;c++)
			{
				if (input_image->image_pixels[i][j][c] + details_image->image_pixels[i][j][c] < 255)
					image->image_pixels[i][j][c] = uint8_t(input_image->image_pixels[i][j][c] + details_image->image_pixels[i][j][c]);
				else
					image->image_pixels[i][j][c] = uint8_t(255);
			}
		}
	}

	return image;
}


void thread_s1(struct image_t *input_image, struct image_t **smoothened_image){
    *smoothened_image = S1_smoothen(input_image);
    Is_s1_done.store(true);
}

void thread_s2(struct image_t *input_image, struct image_t **smoothened_image, struct image_t **details_image){
    // thread s2 will wait until s1 completes
    while (!Is_s1_done.load()) { 
        this_thread::yield(); 
    }
    *details_image = S2_find_details(input_image, *smoothened_image);
    Is_s2_done.store(true);
}

void thread_s3(struct image_t *input_image, struct image_t **details_image, struct image_t **sharpened_image){
    // thread s3 will wait until s2 completes 
    while (!Is_s2_done.load()) { 
        this_thread::yield(); 
    }
    *sharpened_image = S3_sharpen(input_image, *details_image);
}

void free_image(struct image_t *image){
    if(image!=nullptr){
        for(int i=0;i<image->height;i++){
            for(int j=0;j<image->width;j++){
                delete[] image->image_pixels[i][j]; 
            }
            delete[] image->image_pixels[i]; 
        }
        delete[] image->image_pixels; 
        delete image; 
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
        exit(0);
    }

    chrono::time_point<std::chrono::high_resolution_clock> start, end;
    start = chrono::high_resolution_clock::now();
    
    struct image_t *input_image = read_ppm_file(argv[1]);

    struct image_t *smoothened_image;
    struct image_t *details_image;
    struct image_t *sharpened_image;

    for(int i=0; i<1000; i++){
        // in the starting of each iterations, we will lock s2 and s3 from making any further progress until s1 and s2 are completed resp.
        Is_s1_done.store(false);
        Is_s2_done.store(false);

        // creating threads for S1, S2 and S3
        thread t1(thread_s1, input_image, &smoothened_image);
        thread t2(thread_s2, input_image, &smoothened_image, &details_image);
        thread t3(thread_s3, input_image, &details_image, &sharpened_image);

        t1.join();
        t2.join();
        t3.join();
        // using .join(), we will wait for each thread to be finished.
		free_image(smoothened_image);
		free_image(details_image);
		if(i!=999){
			free_image(sharpened_image);
		}
    }

    write_ppm_file(argv[2], sharpened_image);
	free_image(sharpened_image);
    end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;

    return 0;
}