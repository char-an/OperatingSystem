#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <ctime>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

using namespace std;

#define SIZE 3

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

    for (int i = 0; i < input_image->height; i++)
    {
        for (int j = 0; j < input_image->width; j++)
        {
            if (i == 0 || j == 0 || i == input_image->height - 1 || j == input_image->width - 1)
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
    if (shm_fd == -1)
    {
        perror("shm_open");
        return 1;
    }

    size_t size = input_image->height * input_image->width * 3 + 10;
    cout << "Calculated size for shared memory: " << size << endl;

    // Resize the shared memory object to the desired size
    if (ftruncate(shm_fd, size) == -1)
    {
        perror("ftruncate");
        return 1;
    }

    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    sem_t *sem1 = sem_open("/my_semaphore1", O_CREAT, 0666, 1);

    if (sem1 == SEM_FAILED)
    {
        perror("semaphore open failed");
        return 1;
    }

    int tmp;

    // write data to the shared memory
    for (int count = 0; count < SIZE; count++)
    {
        cout << "inside count s1" << endl;

        smoothened_image = S1_smoothen(input_image);
        cout << "inside smooth s1" << endl;
        if (count != 0)
        {
            tmp = 0;
            while (tmp == 0)
            {
                cout << "inside while s1" << endl;
                sem_wait(sem1);
                memcpy(&tmp, ptr + sizeof(int), sizeof(int));
                sem_post(sem1);
            }
        }

        cout << "outside sem s1" << endl;
        sem_wait(sem1);
        cout << "inside sem s1" << endl;

        for (int i = 0; i < smoothened_image->height; i++)
        {
            for (int j = 0; j < smoothened_image->width; j++)
            {
                memcpy((char *)ptr + (i * smoothened_image->width + j) * 3 + 10, smoothened_image->image_pixels[i][j], 3);
            }
        }
        cout << "Image written to shared memory successfully! (" << count + 1 << "/1000)" << endl;

        tmp = 1;
        memcpy(ptr, &tmp, sizeof(int));

        tmp = 0;
        memcpy(&tmp, ptr + sizeof(int), sizeof(int));
        sem_post(sem1);
    }

    // write_ppm_file(argv[2], sharpened_image);

    end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;

    if (sem_close(sem1))
    {
        perror("sem_close");
        return 1;
    }

    if (sem_unlink("/my_semaphore1"))
    {
        perror("sem_unlink");
        return 1;
    }
    // if(sem_close(sem2)){
    // 	perror("sem_close");
    // 	return 1;
    // }

    // if(sem_unlink("/my_semaphore2")){
    // 	perror("sem_unlink");
    // 	return 1;
    // }

    // Unmap the shared memory object
    if (munmap(ptr, size) == -1)
    {
        perror("munmap");
        return 1;
    }

    // Close the shared memory file descriptor
    if (close(shm_fd) == -1)
    {
        perror("close");
        return 1;
    }

    // Unlink the shared memory object
    if (shm_unlink(name) == -1)
    {
        perror("shm_unlink");
        return 1;
    }

    return 0;
}