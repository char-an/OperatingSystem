#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <ctime>

using namespace std;

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

	for (int i = 0; i < 1000; i++)
	{
		S1_smoothen(input_image);
		S2_find_details(input_image);
		S3_sharpen(input_image);
	}

	write_ppm_file(argv[2], sharpened_image);
	free_image(smoothened_image);
	free_image(details_image);
	free_image(sharpened_image);

	end = chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed_seconds = end - start;
	cout << "time taken: " << elapsed_seconds.count() << " seconds" << endl;
	return 0;
}