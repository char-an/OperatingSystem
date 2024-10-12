#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <ctime>
#include<sys/mman.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>

using namespace std;

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

    const char *name = "/shared_s23";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("shm_open");
        return 1;
    }

    size_t size = input_image->height * input_image->width * 3;
	cout << "Calculated size for shared memory: " << size << endl;

    // Resize the shared memory object to the desired size
    if (ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        return 1;
    }

    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
	}
    

    // Cast ptr to uint8_t* for indexing
    uint8_t* shared_memory = static_cast<uint8_t*>(ptr);

    // Reconstruct smoothened_image from shared memory
    details_image = new image_t;
    details_image->height = input_image->height;
    details_image->width = input_image->width;
    details_image->image_pixels = new uint8_t** [details_image->height];
    for (int i = 0; i < details_image->height; i++)
    {
        details_image->image_pixels[i] = new uint8_t* [details_image->width];
        for (int j = 0; j < details_image->width; j++)
        {
            details_image->image_pixels[i][j] = new uint8_t[3];

            // Read RGB values from shared memory
            int index = (i * details_image->width + j) * 3;
            details_image->image_pixels[i][j][0] = shared_memory[index];     // Red
            details_image->image_pixels[i][j][1] = shared_memory[index + 1]; // Green
            details_image->image_pixels[i][j][2] = shared_memory[index + 2]; // Blue
        }
    }

    sharpened_image = S3_sharpen(input_image,details_image);

    cout << "Image read from shared memory successfully!\n";
	write_ppm_file(argv[2], sharpened_image);

	end = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;

    // Unmap the shared memory object
    if (munmap(ptr, size) == -1) {
        perror("munmap");
        return 1;
    }

    // Close the shared memory file descriptor
    if (close(shm_fd) == -1) {
        perror("close");
        return 1;
    }

    // Unlink the shared memory object
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        return 1;
    }

	return 0;
}