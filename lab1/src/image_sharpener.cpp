#include <iostream>
#include "libppm.h"
#include <cstdint>

using namespace std;

struct image_t *S1_smoothen(struct image_t *input_image)
{
	// TODO
	// remember to allocate space for smoothened_image. See read_ppm_file() in libppm.c for some help.

	// when image is null
	if (input_image == nullptr)
		return nullptr;

	image_t *img = new image_t;
	img->width = input_image->width;
	img->height = input_image->height;
	img->image_pixels = new uint8_t **[input_image->height];

	for (int i = 0; i < input_image->height; ++i)
	{
		img->image_pixels[i] = new uint8_t *[input_image->width];
		for (int j = 0; j < input_image->width; ++j)
		{
			img->image_pixels[i][j] = new uint8_t[3]; // For RGB components
		}
	}

	int delrow[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
	int delcol[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};

	for (int i = 0; i < input_image->height; ++i)
	{
		img->image_pixels[i] = new uint8_t *[input_image->width];
		for (int j = 0; j < input_image->width; ++j)
		{
			if (i == 0 || j == 0 || i == input_image->height - 1 || j == input_image->width - 1)
			{
				img->image_pixels[i][j][0] = input_image->image_pixels[i][j][0];
				img->image_pixels[i][j][1] = input_image->image_pixels[i][j][1];
				img->image_pixels[i][j][2] = input_image->image_pixels[i][j][2];
				continue;
			}
			// for red
			for (int k = 0; k < 9; k++)
			{
				img->image_pixels[i][j][0] += 1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][0]);
			}
			// for green
			for (int k = 0; k < 9; k++)
			{
				img->image_pixels[i][j][1] += 1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][1]);
			}
			// for blue
			for (int k = 0; k < 9; k++)
			{
				img->image_pixels[i][j][2] += 1.0 / 9 * (input_image->image_pixels[i + delrow[k]][j + delcol[k]][2]);
			}
		}
	}

	return img;
}

struct image_t *S2_find_details(struct image_t *input_image, struct image_t *smoothened_image)
{
	// TODO

	return 0;
}

struct image_t *S3_sharpen(struct image_t *input_image, struct image_t *details_image)
{
	// TODO
	return input_image; // TODO remove this line when adding your code
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
		exit(0);
	}

	struct image_t *input_image = read_ppm_file(argv[1]);

	struct image_t *smoothened_image = S1_smoothen(input_image);

	struct image_t *details_image = S2_find_details(input_image, smoothened_image);

	struct image_t *sharpened_image = S3_sharpen(input_image, details_image);

	write_ppm_file(argv[2], smoothened_image);

	return 0;
}
