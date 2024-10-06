#include <iostream>
#include "libppm.h"
#include <cstdint>
#include <cmath>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


using namespace std;

void send_image(int pipefd, struct image_t *image) {
    cout << "Sending Image: Height = " << image->height << ", Width = " << image->width << endl;

    write(pipefd, &(image->height), sizeof(int));
    write(pipefd, &(image->width), sizeof(int));

    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            write(pipefd, image->image_pixels[i][j], 3 * sizeof(uint8_t)); //RGB
        }
    }
}

struct image_t *receive_image(int pipefd) {
    struct image_t *image = new struct image_t;

    read(pipefd, &(image->height), sizeof(int));
    read(pipefd, &(image->width), sizeof(int));

    cout << "Receiving Image: Height = " << image->height << ", Width = " << image->width << endl;

    image->image_pixels = new uint8_t **[image->height];
    for (int i = 0; i < image->height; i++) {
        image->image_pixels[i] = new uint8_t *[image->width];
        for (int j = 0; j < image->width; j++) {
            image->image_pixels[i][j] = new uint8_t[3];
            read(pipefd, image->image_pixels[i][j], 3 * sizeof(uint8_t)); //RGB
        }
    }
    return image;
}

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

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << "usage: ./a.out <path-to-original-image> <path-to-transformed-image>\n\n";
		exit(0);
	}
    
	struct image_t *input_image = read_ppm_file(argv[1]);

	struct image_t *smoothened_image;
	struct image_t *details_image;
	struct image_t *sharpened_image;

	//pipe for s1 to s2
	int pipefd1[2];
	pipe(pipefd1);

	pid_t child = fork();

    if(child > 0){ // process1
	
		close(pipefd1[0]);
        smoothened_image = S1_smoothen(input_image);
		cout << "Parent: Smoothened image created, sending to child..." << getpid() << endl;
		send_image(pipefd1[1], smoothened_image);
		close(pipefd1[1]);

		wait(NULL); 
    }
    else{
		//pipe for s2 to s3
		int pipefd2[2]; 
		pipe(pipefd2);

        pid_t grandchild = fork();

        if(grandchild > 0){ // process2
			close(pipefd1[1]);
			close(pipefd2[0]);

			smoothened_image = receive_image(pipefd1[0]); // Receive smoothened image data
            details_image = S2_find_details(input_image, smoothened_image);
			cout << "Child: Details image created, sending to grandchild..." << getpid() << endl;
            send_image(pipefd2[1], details_image); // Send details image data

            close(pipefd1[0]); // Close the read end after receiving
            close(pipefd2[1]); // Close the write end after sending		

			wait(NULL); 
        }
        else{ 				// process3
			close(pipefd2[1]);

			details_image = receive_image(pipefd2[0]); // Receive details image data
            sharpened_image = S3_sharpen(input_image, details_image);
			cout << "Grandchild: Sharpened image created, writing to file..." << getpid() << endl;
            write_ppm_file(argv[2], sharpened_image); // Write the final sharpened image

            close(pipefd2[0]);

        }
    }
	cout << "Process ID: " << getpid() << " is finishing." << endl;
	return 0;
}
