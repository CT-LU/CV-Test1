#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;
using namespace cv;

int main( int argc, char** argv )
{

	Mat src, src_gray;
	Mat clone_src_gray1; // used in implementing GaussianBlur 
	Mat clone_src_gray2; // used in implementing Sobel operation

	int GaussianKernel[5][5];
	double n, pdf_0, pdf_1, pdf_2, pi = 3.14159;
	double mean_num;

	cout << "*****************************************************************" << endl;	
	cout << "the used Gaussian distribution: sigma = 1, mean value = 0 " << endl;
	cout << "*****************************************************************" << endl;	
	pdf_0 = 1/sqrt(2*pi)*exp(-0); // -0 = -0*0/2
	pdf_1 = 1/sqrt(2*pi)*exp(-0.5); // -0.5 = -1*1/2
	pdf_2 = 1/sqrt(2*pi)*exp(-2); // -2 = -2*2/2

	cout << "pdf_0 = " << pdf_0 << endl;
	cout << "pdf_1 = " << pdf_1 << endl;
	cout << "pdf_2 = " << pdf_2 << endl;
	cout << "*****************************************************************" << endl;	
	
	//***************************************************
	mean_num = atoi(argv[2]);
	//***************************************************

	//    00  01  02  03  04
	//    10  11  12  13  14
	//    20  21  22  23  24
	//    30  31  32  33  34
	//    40  41  42  43  44

	GaussianKernel[2][2] = mean_num;
	GaussianKernel[1][2] = mean_num/pdf_0*pdf_1;
	GaussianKernel[3][2] = mean_num/pdf_0*pdf_1;
	GaussianKernel[0][2] = mean_num/pdf_0*pdf_2;
	GaussianKernel[4][2] = mean_num/pdf_0*pdf_2;

	GaussianKernel[0][1] = GaussianKernel[0][2]/pdf_0*pdf_1;
	GaussianKernel[0][3] = GaussianKernel[0][2]/pdf_0*pdf_1;
	GaussianKernel[0][0] = GaussianKernel[0][2]/pdf_0*pdf_2;
	GaussianKernel[0][4] = GaussianKernel[0][2]/pdf_0*pdf_2;

	GaussianKernel[1][1] = GaussianKernel[1][2]/pdf_0*pdf_1;
	GaussianKernel[1][3] = GaussianKernel[1][2]/pdf_0*pdf_1;
	GaussianKernel[1][0] = GaussianKernel[1][2]/pdf_0*pdf_2;
	GaussianKernel[1][4] = GaussianKernel[1][2]/pdf_0*pdf_2;

	GaussianKernel[2][1] = GaussianKernel[2][2]/pdf_0*pdf_1;
	GaussianKernel[2][3] = GaussianKernel[2][2]/pdf_0*pdf_1;
	GaussianKernel[2][0] = GaussianKernel[2][2]/pdf_0*pdf_2;
	GaussianKernel[2][4] = GaussianKernel[2][2]/pdf_0*pdf_2;

	GaussianKernel[3][1] = GaussianKernel[3][2]/pdf_0*pdf_1;
	GaussianKernel[3][3] = GaussianKernel[3][2]/pdf_0*pdf_1;
	GaussianKernel[3][0] = GaussianKernel[3][2]/pdf_0*pdf_2;
	GaussianKernel[3][4] = GaussianKernel[3][2]/pdf_0*pdf_2;

	GaussianKernel[4][1] = GaussianKernel[4][2]/pdf_0*pdf_1;
	GaussianKernel[4][3] = GaussianKernel[4][2]/pdf_0*pdf_1;
	GaussianKernel[4][0] = GaussianKernel[4][2]/pdf_0*pdf_2;
	GaussianKernel[4][4] = GaussianKernel[4][2]/pdf_0*pdf_2;

	int Kernel_sum = 0;
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<5; j++)
		{
			Kernel_sum = Kernel_sum + GaussianKernel[i][j];
		}
	}
	cout << "Kernel_sum = " << Kernel_sum << endl;
	cout << "*****************************************************************" << endl;	


	/// Load an image
	//***************************************************
	src = imread( argv[1] );
	//***************************************************


	if( !src.data )
	{ return -1; }

#if 0	
	GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
#endif

	cvtColor( src, src_gray, CV_RGB2GRAY );

	//Implement GaussianBlur

	clone_src_gray1 = src_gray.clone();

	for(int y = 0; y < clone_src_gray1.rows; y++)
		for(int x = 0; x < clone_src_gray1.cols; x++)
			clone_src_gray1.at<uchar>(y,x) = 0.0;

	int clone_src_Blur;
	for(int y = 2; y < clone_src_gray1.rows - 2; y++){
		for(int x = 2; x < clone_src_gray1.cols - 2; x++){
			clone_src_Blur = (GaussianKernel[0][0]*src_gray.at<uchar>(y-2, x-2) +
					GaussianKernel[1][0]*src_gray.at<uchar>(y-1, x-2) +
					GaussianKernel[2][0]*src_gray.at<uchar>(y, x-2) +
					GaussianKernel[3][0]*src_gray.at<uchar>(y+1, x-2) +
					GaussianKernel[4][0]*src_gray.at<uchar>(y+2, x-2) +
					GaussianKernel[0][1]*src_gray.at<uchar>(y-2, x-1) +
					GaussianKernel[1][1]*src_gray.at<uchar>(y-1, x-1) +
					GaussianKernel[2][1]*src_gray.at<uchar>(y, x-1) +
					GaussianKernel[3][1]*src_gray.at<uchar>(y+1, x-1) +
					GaussianKernel[4][1]*src_gray.at<uchar>(y+2, x-1) +
					GaussianKernel[0][2]*src_gray.at<uchar>(y-2, x) +
					GaussianKernel[1][2]*src_gray.at<uchar>(y-1, x) +
					GaussianKernel[2][2]*src_gray.at<uchar>(y, x) +
					GaussianKernel[3][2]*src_gray.at<uchar>(y+1, x) +
					GaussianKernel[4][2]*src_gray.at<uchar>(y+2, x) +
					GaussianKernel[0][3]*src_gray.at<uchar>(y-2, x+1) +
					GaussianKernel[1][3]*src_gray.at<uchar>(y-1, x+1) +
					GaussianKernel[2][3]*src_gray.at<uchar>(y, x+1) +
					GaussianKernel[3][3]*src_gray.at<uchar>(y+1, x+1) +
					GaussianKernel[4][3]*src_gray.at<uchar>(y+2, x+1) +
					GaussianKernel[0][4]*src_gray.at<uchar>(y-2, x+2) +
					GaussianKernel[1][4]*src_gray.at<uchar>(y-1, x+2) +
					GaussianKernel[2][4]*src_gray.at<uchar>(y, x+2) +
					GaussianKernel[3][4]*src_gray.at<uchar>(y+1, x+2) +
					GaussianKernel[4][4]*src_gray.at<uchar>(y+2, x+2))/Kernel_sum; 
			clone_src_gray1.at<uchar>(y,x) = clone_src_Blur;
		}
	}
	imshow("Not GaussianBlur image", src_gray);
	imshow("GaussianBlur image", clone_src_gray1);


	//Implenent the Sobel 
	//             1 0 -1               1  2  1
	// x_kernel =  2 0 -2   y_kernel =  0  0  0
	//             1 0 -1              -1 -2 -1
	clone_src_gray2 = clone_src_gray1.clone();

	for(int y = 0; y < src_gray.rows; y++)
		for(int x = 0; x < src_gray.cols; x++)
			clone_src_gray2.at<uchar>(y,x) = 0.0;

	int grad_x, grad_y;
	int sum_gray;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	for(int y = 1; y < src_gray.rows - 1; y++){
		for(int x = 1; x < src_gray.cols - 1; x++){
			grad_x = clone_src_gray1.at<uchar>(y-1, x-1) +
				2*clone_src_gray1.at<uchar>(y, x-1) +
				clone_src_gray1.at<uchar>(y+1, x-1) -
				clone_src_gray1.at<uchar>(y-1, x+1) -
				2*clone_src_gray1.at<uchar>(y, x+1) -
				clone_src_gray1.at<uchar>(y+1, x+1);
			grad_y = clone_src_gray1.at<uchar>(y-1, x-1) +
				2*clone_src_gray1.at<uchar>(y-1, x) +
				clone_src_gray1.at<uchar>(y-1, x+1) -
				clone_src_gray1.at<uchar>(y+1, x-1) -
				2*clone_src_gray1.at<uchar>(y+1, x) -
				clone_src_gray1.at<uchar>(y+1, x+1);
			sum_gray = (abs(grad_x) + abs(grad_y));
			sum_gray = sum_gray > 255 ? 255:sum_gray;
			if(sum_gray > 100){
				sum_gray = 255;			
			}else{
				sum_gray = 0;				
			}
			clone_src_gray2.at<uchar>(y,x) = sum_gray;
		}
	}
        imshow("Sobel image", clone_src_gray2);
	findContours( clone_src_gray2, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
	Mat drawing = Mat::zeros( clone_src_gray2.size(), CV_8UC3 );
	RNG rng(12345);
	for( int i = 0; i< contours.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
	}

	/// Show in a window
	imshow( "Contours", drawing );

	waitKey(0);

	return 0;
}

