#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ppm/ppm.h"

// RAD is the radius of the region of support for the search
#define scale 1		// 1 for 900 * 750, 2 for 1800 * 1500 for instance  
#define RAD (8 * scale) 

void block_stereo_matching(unsigned int *left, unsigned int *right, unsigned int *depthData,
		int w, int h, int minDisparity, int maxDisparity)
{
	for (int y = 0 ; y < h ; y++)
	{
		for (int x = 0 ; x < w ; x++)
		{
			unsigned int bestCost = 9999999;
			unsigned int bestDisparity = 0;

			for (int d = minDisparity; d <= maxDisparity; d++)
			{
				unsigned int cost = 0;

				for (int i = -RAD; i <= RAD; i++)
					for (int j = -RAD; j <= RAD; j++)
					{
						//border clamping
						int yy, xx, xxd;
						yy = y + i;

						if (yy < 0) yy = 0;

						if (yy >= h) yy = h - 1;

						xx = x + j;

						if (xx < 0) xx = 0;

						if (xx >= w) xx = w - 1;

						xxd = x + j + d;

						if (xxd < 0) xxd = 0;

						if (xxd >= w) xxd = w - 1;

						// sum abs diff across components
						unsigned char *leftPixel = (unsigned char *)&left[yy*w + xx];
						unsigned char *rightPixel = (unsigned char *)&right[yy*w + xxd];
						unsigned int absdiff = 0;

						for (int k = 0; k < 3; k++) // R.G.B.X ingnore X
						{
							absdiff += abs((int)(leftPixel[k] - rightPixel[k]));
						}

						cost += absdiff; 
					}

				if (cost < bestCost)
				{
					bestCost = cost;
					bestDisparity = d * 7 / scale;
				}

			}// end for disparities

			// store to best disparity
			depthData[y*w + x ] = bestDisparity;
		}
	}
}


int main(int argc, char **argv)
{
	// Search ranges 
	int minDisp = -32 * scale;
	int maxDisp = 0;
	
	//PPMImage *leftImg = readPPM("data/stereo.im7.1800x1500.ppm");
	//PPMImage *rightImg = readPPM("data/stereo.im8.1800x1500.ppm");
	PPMImage *leftImg = readPPM("data/stereo.im0.900x750.ppm");
	PPMImage *rightImg = readPPM("data/stereo.im1.900x750.ppm");
	// each pixel of PPM images has 4 byte memory (RGBX)

	unsigned int w = leftImg->x;
	unsigned int h = leftImg->y;

	printf("w:%d, h:%d\n", w, h);
	unsigned int numData = w * h;
	unsigned int memSize = sizeof(unsigned int) * numData;
	unsigned int *outData = (unsigned int *)malloc(memSize);

	memset(outData, 0x00, memSize);

	// write out the resulting disparity image.
	unsigned int *dispOut = (unsigned int *)malloc(memSize);

	block_stereo_matching((unsigned int *)leftImg->data, (unsigned int *)rightImg->data, (unsigned int *)outData, w, h, minDisp, maxDisp);

	for (unsigned int i = 0; i < numData; i++)
	{
		unsigned char* byte = (unsigned char*)&outData[i];
		byte[2] = byte[1] = byte[0] = outData[i]; // transfer to gray
	}

	const char *cpuFnameOut = "depth_image.ppm";

	free(leftImg->data);
	leftImg->data = outData;
	printf("Output image: <%s>\n", cpuFnameOut);
	writePPM(cpuFnameOut, leftImg);

	free(leftImg->data);
	free(rightImg->data);
	free(leftImg);
	free(rightImg);

	return -1;
}
