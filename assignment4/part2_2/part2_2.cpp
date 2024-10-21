#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <cstring>

#define SIZE 1000

using namespace std;

struct image_t *smoothened_image;

void S1_smoothen(struct image_t *input_image)
{
    if (input_image == nullptr)
    {
        cout << "Input image in smoothen function is null" << endl;
        return;
    }

    int delrow[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    int delcol[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};

    for (int i = 0; i < input_image->height; i++)
    {
        for (int j = 0; j < input_image->width; j++)
        {
            if (i == 0 || j == 0 || i == input_image->height - 1 || j == input_image->width - 1)
            {
                smoothened_image->image_pixels[i][j][0] = input_image->image_pixels[i][j][0];
                smoothened_image->image_pixels[i][j][1] = input_image->image_pixels[i][j][1];
                smoothened_image->image_pixels[i][j][2] = input_image->image_pixels[i][j][2];
                continue;
            }

            smoothened_image->image_pixels[i][j][0] = 0;
            smoothened_image->image_pixels[i][j][1] = 0;
            smoothened_image->image_pixels[i][j][2] = 0;

            for (int k = 0; k < 9; k++)
            {
                smoothened_image->image_pixels[i][j][0] += uint8_t(1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][0]));
                smoothened_image->image_pixels[i][j][1] += uint8_t(1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][1]));
                smoothened_image->image_pixels[i][j][2] += uint8_t(1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][2]));
            }
        }
    }
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
    for (int i = 0; i < input_image->height; i++)
    {
        for (int j = 0; j < input_image->width; j++)
        {
            for (int c = 0; c < 3; c++)
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
    for (int i = 0; i < input_image->height; i++)
    {
        for (int j = 0; j < input_image->width; j++)
        {
            for (int c = 0; c < 3; c++)
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

void free_image(struct image_t *image)
{
    if (image != nullptr)
    {
        for (int i = 0; i < image->height; i++)
        {
            for (int j = 0; j < image->width; j++)
            {
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

    // struct image_t *smoothened_image;
    // struct image_t *details_image;
    // struct image_t *sharpened_image;

    pid_t child = fork();

    if (child > 0)
    { // process1  - s1

        smoothened_image = new struct image_t;
        smoothened_image->height = input_image->height;
        smoothened_image->width = input_image->width;
        smoothened_image->image_pixels = new uint8_t **[input_image->height];
        for (int i = 0; i < input_image->height; i++)
        {
            smoothened_image->image_pixels[i] = new uint8_t *[input_image->width];
            for (int j = 0; j < input_image->width; j++)
            {
                smoothened_image->image_pixels[i][j] = new uint8_t[3];
            }
        }

        const char *name = "/shared_s12";
        int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1)
        {
            perror("shm_open");
            return 1;
        }

        size_t size = input_image->height * input_image->width * 3 + 10;
        // cout << "Calculated size for shared memory: " << size << endl;

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
            // cout << "inside count s1" << endl;

            // struct image_t *smoothened_image = S1_smoothen(input_image);
            S1_smoothen(input_image);
            // cout << "inside smooth s1" << endl;
            if (count != 0)
            {
                tmp = 0;
                while (tmp == 0)
                {
                    // cout << "inside while s1" << endl;
                    sem_wait(sem1);
                    memcpy(&tmp, ptr + sizeof(int), sizeof(int));
                    sem_post(sem1);
                }
            }

            // cout << "outside sem s1" << endl;
            sem_wait(sem1);
            // cout << "inside sem s1" << endl;

            for (int i = 0; i < smoothened_image->height; i++)
            {
                for (int j = 0; j < smoothened_image->width; j++)
                {
                    memcpy((char *)ptr + (i * smoothened_image->width + j) * 3 + 10, smoothened_image->image_pixels[i][j], 3);
                }
            }
            // cout << "Image written to shared memory successfully by S1! (" << count + 1 << "/1000)" << endl;

            tmp = 1;
            memcpy(ptr, &tmp, sizeof(int));

            tmp = 0;
            memcpy(ptr + sizeof(int), &tmp, sizeof(int));
            sem_post(sem1);

            // if(count%5==0){
            // free_image(smoothened_image);
            // }
        }

        // write_ppm_file(argv[2], sharpened_image);

        // end = chrono::high_resolution_clock::now();
        // chrono::duration<double> elapsed_seconds = end - start;
        // cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;

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
        wait(NULL);
        // Unlink the shared memory object
        if (shm_unlink(name) == -1)
        {
            perror("shm_unlink");
            return 1;
        }

        end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed_seconds = end - start;

        cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;
    }
    else
    {
        pid_t grandchild = fork();

        if (grandchild > 0)
        { // process2  - s2

            const char *name = "/shared_s12";
            int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
            if (shm_fd == -1)
            {
                perror("shm_open");
                return 1;
            }

            size_t size = input_image->height * input_image->width * 3 + 10;
            // cout << "Calculated size for shared memory: " << size << endl;

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

            const char *name2 = "/shared_s23";
            int shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);
            if (shm_fd2 == -1)
            {
                perror("shm_open");
                return 1;
            }

            // Resize the shared memory object to the desired size
            if (ftruncate(shm_fd2, size) == -1)
            {
                perror("ftruncate");
                return 1;
            }

            void *ptr2 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd2, 0);
            if (ptr2 == MAP_FAILED)
            {
                perror("mmap");
                return 1;
            }

            // Cast ptr to uint8_t* for indexing
            uint8_t *shared_memory = static_cast<uint8_t *>(ptr);

            sem_t *sem1 = sem_open("/my_semaphore1", O_CREAT, 0666, 1);
            sem_t *sem2 = sem_open("/my_semaphore2", O_CREAT, 0666, 1);
            if (sem1 == SEM_FAILED)
            {
                perror("semaphore open failed");
                return 1;
            }
            if (sem2 == SEM_FAILED)
            {
                perror("semaphore open failed");
                return 1;
            }
            int tmp;
            for (int count = 0; count < SIZE; count++)
            {
                // cout << "inside count s2" << endl;

                tmp = 0;
                while (tmp == 0)
                {
                    // cout << "inside while s2" << endl;
                    sem_wait(sem1);
                    memcpy(&tmp, ptr, sizeof(int));
                    sem_post(sem1);
                }
                sem_wait(sem1);
                // cout << "inside sem s2" << endl;
                //  Reconstruct smoothened_image from shared memory
                struct image_t *smoothened_img = new image_t;
                smoothened_img->height = input_image->height;
                smoothened_img->width = input_image->width;
                smoothened_img->image_pixels = new uint8_t **[smoothened_img->height];
                for (int i = 0; i < smoothened_img->height; i++)
                {
                    smoothened_img->image_pixels[i] = new uint8_t *[smoothened_img->width];
                    for (int j = 0; j < smoothened_img->width; j++)
                    {
                        smoothened_img->image_pixels[i][j] = new uint8_t[3];

                        // Read RGB values from shared memory
                        int index = (i * smoothened_img->width + j) * 3 + 10;
                        smoothened_img->image_pixels[i][j][0] = shared_memory[index];     // Red
                        smoothened_img->image_pixels[i][j][1] = shared_memory[index + 1]; // Green
                        smoothened_img->image_pixels[i][j][2] = shared_memory[index + 2]; // Blue
                    }
                }
                // cout << "Image read shared memory successfully by S2! (" << count + 1 << "/1000)" << endl;

                tmp = 1;
                memcpy(ptr + sizeof(int), &tmp, sizeof(int));

                struct image_t *details_image = S2_find_details(input_image, smoothened_img);

                tmp = 0;
                memcpy(ptr, &tmp, sizeof(int));
                // sem_post(sem1);

                if (count != 0)
                {
                    tmp = 0;
                    while (tmp == 0)
                    {
                        // cout << "inside while s2" << endl;
                        sem_wait(sem2);
                        memcpy(&tmp, ptr2 + sizeof(int), sizeof(int));
                        sem_post(sem2);
                    }
                }

                for (int i = 0; i < details_image->height; i++)
                {
                    for (int j = 0; j < details_image->width; j++)
                    {
                        memcpy((char *)ptr2 + (i * details_image->width + j) * 3 + 10, details_image->image_pixels[i][j], 3);
                    }
                }
                // cout << "Image written to shared memory successfully by S2! (" << count + 1 << "/1000)" << endl;

                tmp = 1;
                memcpy(ptr2, &tmp, sizeof(int));

                tmp = 0;
                memcpy(ptr2 + sizeof(int), &tmp, sizeof(int));
                sem_post(sem2);
                sem_post(sem1);

                free_image(smoothened_img);
                free_image(details_image);
            }

            end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed_seconds = end - start;
            // cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;

            if (sem_close(sem1))
            {
                perror("sem_close");
                return 1;
            }

            // if(sem_unlink("/my_semaphore1")){
            //     perror("sem_unlink");
            //     return 1;
            // }
            if (sem_close(sem2))
            {
                perror("sem_close");
                return 1;
            }

            if (sem_unlink("/my_semaphore2"))
            {
                perror("sem_unlink");
                return 1;
            }

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
            // if (shm_unlink(name) == -1) {
            //     perror("shm_unlink");
            //     return 1;
            // }

            // Unmap the shared memory object
            if (munmap(ptr2, size) == -1)
            {
                perror("munmap");
                return 1;
            }

            // Close the shared memory file descriptor
            if (close(shm_fd2) == -1)
            {
                perror("close");
                return 1;
            }
            wait(NULL);
            // Unlink the shared memory object
            if (shm_unlink(name2) == -1)
            {
                perror("shm_unlink");
                return 1;
            }
        }
        else
        { // process3 - s3 + write

            const char *name = "/shared_s23";
            int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
            if (shm_fd == -1)
            {
                perror("shm_open");
                return 1;
            }

            size_t size = input_image->height * input_image->width * 3 + 10;
            // cout << "Calculated size for shared memory: " << size << endl;

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

            sem_t *sem2 = sem_open("/my_semaphore2", O_CREAT, 0666, 1);
            if (sem2 == SEM_FAILED)
            {
                perror("semaphore open failed");
                return 1;
            }

            // Cast ptr to uint8_t* for indexing
            uint8_t *shared_memory = static_cast<uint8_t *>(ptr);

            int tmp;
            for (int count = 0; count < SIZE; count++)
            {
                tmp = 0;
                while (tmp == 0)
                {
                    // cout << "inside while s3" << endl;
                    sem_wait(sem2);
                    memcpy(&tmp, ptr, sizeof(int));
                    sem_post(sem2);
                }
                sem_wait(sem2);

                // Reconstruct smoothened_image from shared memory
                struct image_t *details_image = new image_t;
                details_image->height = input_image->height;
                details_image->width = input_image->width;
                details_image->image_pixels = new uint8_t **[details_image->height];
                for (int i = 0; i < details_image->height; i++)
                {
                    details_image->image_pixels[i] = new uint8_t *[details_image->width];
                    for (int j = 0; j < details_image->width; j++)
                    {
                        details_image->image_pixels[i][j] = new uint8_t[3];

                        // Read RGB values from shared memory
                        int index = (i * details_image->width + j) * 3 + 10;
                        details_image->image_pixels[i][j][0] = shared_memory[index];     // Red
                        details_image->image_pixels[i][j][1] = shared_memory[index + 1]; // Green
                        details_image->image_pixels[i][j][2] = shared_memory[index + 2]; // Blue
                    }
                }

                struct image_t *sharpened_image = S3_sharpen(input_image, details_image);
                // cout << "Image read from shared memory successfully by S3! (" << count + 1 << "/1000)" << endl;

                tmp = 1;
                memcpy(ptr + sizeof(int), &tmp, sizeof(int));

                tmp = 0;
                memcpy(ptr, &tmp, sizeof(int));
                sem_post(sem2);

                free_image(details_image);
                if (count == SIZE - 1)
                {
                    write_ppm_file(argv[2], sharpened_image);
                    free_image(sharpened_image);
                }
                else
                {
                    free_image(sharpened_image);
                }
            }

            // write_ppm_file(argv[2], sharpened_image);

            end = chrono::high_resolution_clock::now();
            chrono::duration<double> elapsed_seconds = end - start;
            // cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;

            if (sem_close(sem2))
            {
                perror("sem_close");
                return 1;
            }

            // if(sem_unlink("/my_semaphore2")){
            //     perror("sem_unlink");
            //     return 1;
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
            // if (shm_unlink(name) == -1) {
            //     perror("shm_unlink");
            //     return 1;
            // }

            // write_ppm_file(argv[2], sharpened_image); // Write the final sharpened image
            // cout << "output image created" << endl;
        }
    }
    // cout << "Process ID: " << getpid() << " is finishing." << endl;
    free_image(input_image);
    free_image(smoothened_image);

    return 0;
}