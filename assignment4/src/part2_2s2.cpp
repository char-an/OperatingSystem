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
#include<semaphore.h>
#include<string.h>

using namespace std;

#define SIZE 3

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

    const char *name = "/shared_s12";
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("shm_open");
        return 1;
    }

    size_t size = input_image->height * input_image->width * 3 ;
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

    const char *name2 = "/shared_s23";
    int shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);
    if(shm_fd2 == -1){
        perror("shm_open");
        return 1;
    }

    size_t size = input_image->height * input_image->width * 3 ;
	cout << "Calculated size for shared memory: " << size << endl;

    // Resize the shared memory object to the desired size
    if (ftruncate(shm_fd2, size) == -1) {
        perror("ftruncate");
        return 1;
    }

    void *ptr2 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
    if (ptr2 == MAP_FAILED) {
        perror("mmap");
        return 1;
	}

    // Cast ptr to uint8_t* for indexing
    uint8_t* shared_memory = static_cast<uint8_t*>(ptr);

    sem_t * sem1= sem_open("/my_semaphore1", O_CREAT, 0666, 1);
	sem_t * sem2= sem_open("/my_semaphore2", O_CREAT, 0666, 1);
	if(sem1 <= 0){
        perror("semaphore open failed");
		return 1;
	}
	if(sem2 <= 0){
        perror("semaphore open failed");
		return 1;
	}

    for(int count =0;count<SIZE;count++){
        // Reconstruct smoothened_image from shared memory
        sem_wait(sem2);
        smoothened_image = new image_t;
        smoothened_image->height = input_image->height;
        smoothened_image->width = input_image->width;
        smoothened_image->image_pixels = new uint8_t** [smoothened_image->height];
        for (int i = 0; i < smoothened_image->height; i++)
        {
            smoothened_image->image_pixels[i] = new uint8_t* [smoothened_image->width];
            for (int j = 0; j < smoothened_image->width; j++)
            {
                smoothened_image->image_pixels[i][j] = new uint8_t[3];

                // Read RGB values from shared memory
                int index = (i * smoothened_image->width + j) * 3;
                smoothened_image->image_pixels[i][j][0] = shared_memory[index];     // Red
                smoothened_image->image_pixels[i][j][1] = shared_memory[index + 1]; // Green
                smoothened_image->image_pixels[i][j][2] = shared_memory[index + 2]; // Blue
            }
        }

        details_image = S2_find_details(input_image,smoothened_image);

        int check;
		sem_getvalue(sem1, &check);
		if(check<1){
			sem_post(sem1);
		}
        else{
            cout << "sem logic error" << endl;
        }
    }

    // for (int i = 0; i < details_image->height; i++)
    // {
    //     for (int j = 0; j < details_image->width; j++)
    //     {
    //        	memcpy((char *)ptr2 + (i * details_image->width + j) * 3, details_image->image_pixels[i][j], 3);
    //     }
    // }
    // cout << "Image written to shared memory successfully!\n";

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

    // Unmap the shared memory object
    if (munmap(ptr2, size) == -1) {
        perror("munmap");
        return 1;
    }

    // Close the shared memory file descriptor
    if (close(shm_fd2) == -1) {
        perror("close");
        return 1;
    }

    // Unlink the shared memory object
    if (shm_unlink(name2) == -1) {
        perror("shm_unlink");
        return 1;
    }
    
	return 0;
}