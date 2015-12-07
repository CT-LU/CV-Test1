#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "sobel.hpp"
#include <iostream>

#define matric(x, y) src_GB_raw_data[(y)*image_width+(x)]

using namespace cv;
using namespace std;

/*  The function of GmaskSum is used to generate a Gaussian mask when we give the center value 
 *  for the Gaussian mask. We also compute the sum of each elements of the Gaussian mask.
 */
int gMaskSum(double &G_center, double &pdf_0, double &pdf_1, double &pdf_2, const double pi, int gMask[5][5]){
	/*____________________________________________________________
	 *
	 *  Probability Density Function (PDF):
	 *                          1                -(x - mu)^2
	 *  f(x; mu, sigma) = --------------- * exp(-------------)
	 *                    sigma*sqrt(2pi)          2*sigma^2
	 *
	 *
	 *____________________________________________________________

	 *
	 *	Standard PDF (sigma = 1, mu = 0):
	 *                          1                -x^2
	 *  f(x; mu, sigma) = --------------- * exp(-------)
	 *                      sqrt(2pi)             2
	 *
	 *____________________________________________________________
	 * */

	//Compute the standard PDF using the function above. 
	pdf_0 = 1/sqrt(2*pi)*exp(-0); 
	pdf_1 = 1/sqrt(2*pi)*exp(-0.5); 
	pdf_2 = 1/sqrt(2*pi)*exp(-2); 

	//We first give the center value of gMask, and then compute other elements' values using PDF.
	gMask[2][2] = G_center;
	gMask[1][2] = G_center/pdf_0*pdf_1;
	gMask[3][2] = G_center/pdf_0*pdf_1;
	gMask[0][2] = G_center/pdf_0*pdf_2;
	gMask[4][2] = G_center/pdf_0*pdf_2;

	gMask[0][1] = gMask[0][2]/pdf_0*pdf_1;
	gMask[0][3] = gMask[0][2]/pdf_0*pdf_1;
	gMask[0][0] = gMask[0][2]/pdf_0*pdf_2;
	gMask[0][4] = gMask[0][2]/pdf_0*pdf_2;

	gMask[1][1] = gMask[1][2]/pdf_0*pdf_1;
	gMask[1][3] = gMask[1][2]/pdf_0*pdf_1;
	gMask[1][0] = gMask[1][2]/pdf_0*pdf_2;
	gMask[1][4] = gMask[1][2]/pdf_0*pdf_2;

	gMask[2][1] = gMask[2][2]/pdf_0*pdf_1;
	gMask[2][3] = gMask[2][2]/pdf_0*pdf_1;
	gMask[2][0] = gMask[2][2]/pdf_0*pdf_2;
	gMask[2][4] = gMask[2][2]/pdf_0*pdf_2;

	gMask[3][1] = gMask[3][2]/pdf_0*pdf_1;
	gMask[3][3] = gMask[3][2]/pdf_0*pdf_1;
	gMask[3][0] = gMask[3][2]/pdf_0*pdf_2;
	gMask[3][4] = gMask[3][2]/pdf_0*pdf_2;

	gMask[4][1] = gMask[4][2]/pdf_0*pdf_1;
	gMask[4][3] = gMask[4][2]/pdf_0*pdf_1;
	gMask[4][0] = gMask[4][2]/pdf_0*pdf_2;
	gMask[4][4] = gMask[4][2]/pdf_0*pdf_2;

	//Compute the sum of Gaussian mask
	int Kernel_sum = 0;
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<5; j++)
		{
			Kernel_sum = Kernel_sum + gMask[i][j];
		}
	}
	return Kernel_sum;
}

/* The function of GaussianBlur is used to blur the image.   
 * We will get the better result of Sobel operation if the image is blurred.
 */
void GaussianBlur(Mat &src_gray, Mat &src_GB, int Kernel_sum, int gMask[5][5], int image_height, int image_width){
	uchar* src_GB_raw_data = src_GB.ptr(0);

	int clone_src_Blur;
	for(int y = 2; y < image_height - 2; y++){
		for(int x = 2; x < image_width - 2; x++){
			clone_src_Blur=
				+gMask[0][0]*matric((x-2),(y-2))+gMask[0][1]*matric((x-1),(y-2))+gMask[0][2]*matric(x,(y-2))+gMask[0][3]*matric((x-1),(y-2))+gMask[0][4]*matric((x+2),(y-2))
				+gMask[1][0]*matric((x-2),(y-1))+gMask[1][1]*matric((x-1),(y-1))+gMask[1][2]*matric(x,(y-1))+gMask[1][3]*matric((x-1),(y-1))+gMask[1][4]*matric((x+2),(y-1))
				+gMask[2][0]*matric((x-2),  y  )+gMask[2][1]*matric((x-1),  y  )+gMask[2][2]*matric( x,  y )+gMask[2][3]*matric((x+1),  y  )+gMask[2][4]*matric((x+2),  y  )
				+gMask[3][0]*matric((x-2),(y+1))+gMask[3][1]*matric((x-1),(y+1))+gMask[3][2]*matric(x,(y+1))+gMask[3][3]*matric((x+1),(y+1))+gMask[3][4]*matric((x+2),(y+1))
				+gMask[4][0]*matric((x-2),(y+2))+gMask[4][1]*matric((x-1),(y+2))+gMask[4][2]*matric(x,(y+2))+gMask[4][3]*matric((x+1),(y+2))+gMask[4][4]*matric((x+2),(y+2));

			clone_src_Blur = clone_src_Blur/Kernel_sum;
			src_GB_raw_data[y*image_width+x] = clone_src_Blur;
		}
	}
}


/* The function of Sobel is used to detect the edge of an image.
*/
void Sobel(Mat &src_GB, Mat &src_Sobel, int image_height, int image_width){
	uchar* src_GB_raw_data = src_GB.ptr(0);
	uchar* src_Sobel_raw_data = src_Sobel.ptr(0);


	int grad_x, grad_y;
	int sum_gray;
	for(int y = 1; y < image_height - 1; y++){
		for(int x = 1; x < image_width - 1; x++){
			//                                                    1 0 -1
			//Using x_kernel to compute grad_x, where x_kernel =  2 0 -2 
			//	                                                  1 0 -1
			grad_x =
				+1*matric((x-1), (y-1)) + 0*matric((x), (y-1)) -1*matric((x+1), (y-1))
				+2*matric((x-1), ( y )) + 0*matric((x), ( y )) -2*matric((x+1), ( y ))
				+1*matric((x-1), (y+1)) + 0*matric((x), (y+1)) -1*matric((x+1), (y+1));

			//                                                    1  2  1
			//Using y_kernel to compute grad_y, where y_kernel =  0  0  0 
			//                                                   -1 -2 -1
			grad_y =
				+1*matric((x-1), (y-1)) + 2*matric((x), (y-1)) +1*matric((x+1), (y-1))
				+0*matric((x-1), (y  )) + 0*matric((x), (y  )) +0*matric((x+1), ( y ))
				-1*matric((x-1), (y+1)) - 2*matric((x), (y+1)) -1*matric((x+1), (y+1));

			//Compute the sum of grad+x and grad_y
			sum_gray = (abs(grad_x) + abs(grad_y));
			sum_gray = sum_gray > 255 ? 255:sum_gray;

			//Give a threshold (100) to compute a binary image
			if(sum_gray > 100){
				sum_gray = 255;
			}else{
				sum_gray = 0;
			}
			src_Sobel_raw_data[y*image_width+x] = sum_gray;

		}
	}
}

