#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <zlib.h>

#define SIZE 1000

using namespace std;

uint32_t hasher(uint8_t ***pixels, int height, int width)
{
	uint32_t hashed = crc32(0L, Z_NULL, 0); // initialize hash method

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			hashed = crc32(hashed, reinterpret_cast<const Bytef *>(pixels[i][j]), 3 * sizeof(uint8_t));
		}
	}
	return hashed;
}

void send_image(int pipefd, struct image_t *image)
{
	// cout << "Sending Image: Height = " << image->height << " Width = " << image->width << endl;

	write(pipefd, &(image->height), sizeof(int));
	write(pipefd, &(image->width), sizeof(int));

	for (int i = 0; i < image->height; i++)
	{
		for (int j = 0; j < image->width; j++)
		{
			write(pipefd, image->image_pixels[i][j], 3 * sizeof(uint8_t));
		}
	}
	uint32_t hashed = hasher(image->image_pixels, image->height, image->width);
	write(pipefd, &hashed, sizeof(uint32_t));
}

struct image_t *receive_image(int pipefd)
{
	struct image_t *image = new struct image_t;

	read(pipefd, &(image->height), sizeof(int));
	read(pipefd, &(image->width), sizeof(int));

	// cout << "Receiving Image: Height = " << image->height << " Width = " << image->width << endl;

	image->image_pixels = new uint8_t **[image->height];
	for (int i = 0; i < image->height; i++)
	{
		image->image_pixels[i] = new uint8_t *[image->width];
		for (int j = 0; j < image->width; j++)
		{
			image->image_pixels[i][j] = new uint8_t[3];
			read(pipefd, image->image_pixels[i][j], 3 * sizeof(uint8_t));
		}
	}
	uint32_t get_hashed;
	read(pipefd, &get_hashed, sizeof(uint32_t));
	uint32_t curr_hashed = hasher(image->image_pixels, image->height, image->width);
	if (get_hashed != curr_hashed)
		cout << "image corrupted" << endl;
	// else cout << "image intact" << endl;
	// not sure what logic to implement incase of failed pipe so left it empty as it was not specified to me what to do ;]
	return image;
}

struct image_t *smoothened_image;
struct image_t *details_image;
struct image_t *sharpened_image;

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

void S2_find_details(struct image_t *input_image)
{
	// error handling for nullptr
	if (input_image == nullptr)
	{
		cout << "input image in find_details function is null" << endl;
	}

	// logic
	for (int i = 0; i < input_image->height; i++)
	{
		for (int j = 0; j < input_image->width; j++)
		{
			for (int c = 0; c < 3; c++)
			{
				details_image->image_pixels[i][j][c] = abs(input_image->image_pixels[i][j][c] - smoothened_image->image_pixels[i][j][c]);
			}
		}
	}

	// return image;
}

void S3_sharpen(struct image_t *input_image)
{
	// error handling for nullptr
	if (input_image == nullptr)
	{
		cout << "input image in sharpen function is null" << endl;
	}

	// logic
	for (int i = 0; i < input_image->height; i++)
	{
		for (int j = 0; j < input_image->width; j++)
		{
			for (int c = 0; c < 3; c++)
			{
				if (input_image->image_pixels[i][j][c] + details_image->image_pixels[i][j][c] < 255)
					sharpened_image->image_pixels[i][j][c] = uint8_t(input_image->image_pixels[i][j][c] + details_image->image_pixels[i][j][c]);
				else
					sharpened_image->image_pixels[i][j][c] = uint8_t(255);
			}
		}
	}
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

	details_image = new struct image_t;
	details_image->height = input_image->height;
	details_image->width = input_image->width;
	details_image->image_pixels = new uint8_t **[input_image->height];
	for (int i = 0; i < input_image->height; i++)
	{
		details_image->image_pixels[i] = new uint8_t *[input_image->width];
		for (int j = 0; j < input_image->width; j++)
		{
			details_image->image_pixels[i][j] = new uint8_t[3];
		}
	}

	sharpened_image = new struct image_t;
	sharpened_image->height = input_image->height;
	sharpened_image->width = input_image->width;
	sharpened_image->image_pixels = new uint8_t **[input_image->height];
	for (int i = 0; i < input_image->height; i++)
	{
		sharpened_image->image_pixels[i] = new uint8_t *[input_image->width];
		for (int j = 0; j < input_image->width; j++)
		{
			sharpened_image->image_pixels[i][j] = new uint8_t[3];
		}
	}

	// pipe for s1 to s2
	int pipefd1[2];
	pipe(pipefd1);

	pid_t child = fork();

	if (child > 0)
	{ // process1  - s1

		// chrono::time_point<std::chrono::high_resolution_clock> start, end;
		// start = chrono::high_resolution_clock::now();

		// struct image_t *input_image = read_ppm_file(argv[1]);

		close(pipefd1[0]);

		for (int i = 0; i < SIZE; i++)
		{
			S1_smoothen(input_image);
			// cout << "Parent: Smoothened image created "<< i+1 << " - " << getpid() << endl;
			send_image(pipefd1[1], smoothened_image);
		}

		close(pipefd1[1]);

		wait(NULL);
		end = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed_seconds = end - start;

		cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;
	}
	else
	{
		// pipe for s2 to s3
		int pipefd2[2];
		pipe(pipefd2);

		pid_t grandchild = fork();

		if (grandchild > 0)
		{ // process2  - s2
			close(pipefd1[1]);
			close(pipefd2[0]);

			for (int i = 0; i < SIZE; i++)
			{
				smoothened_image = receive_image(pipefd1[0]); // Receive smoothened image data
				S2_find_details(input_image);
				// cout << "Child: Details image created " << i + 1 << " - " << getpid() << endl;
				free_image(smoothened_image);
				send_image(pipefd2[1], details_image); // Send details image data
			}

			close(pipefd1[0]); // Close the read end after receiving
			close(pipefd2[1]); // Close the write end after sending

			wait(NULL);
		}
		else
		{ // process3 - s3 + write
			close(pipefd2[1]);

			for (int i = 0; i < SIZE; i++)
			{
				details_image = receive_image(pipefd2[0]); // Receive details image data
				S3_sharpen(input_image);
				// cout << "Grandchild: Sharpened image created " << i + 1 << " - " << getpid() << endl;
				free_image(details_image);
			}

			write_ppm_file(argv[2], sharpened_image); // Write the final sharpened image

			close(pipefd2[0]);
		}
	}
	// cout << "Process ID: " << getpid() << " is finishing." << endl;

	free_image(smoothened_image);
	free_image(details_image);
	free_image(sharpened_image);

	return 0;
}