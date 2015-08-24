#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
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

	mean_num = atoi(argv[2]);
	//cout << "please enter the numbers in the location of mean : " ;
	//cin >> mean_num;

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

	int Kernel_sum = 0, num;
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<5; j++)
		{
			Kernel_sum = Kernel_sum + GaussianKernel[i][j];
		}
	}
	cout << "Kernel_sum = " << Kernel_sum << endl;
	cout << "*****************************************************************" << endl;	

	cout << "please enter 1 to continue perform the Sobel optation : ";
	cin >> num;


#if 0
	//double n;
	n = exp(1);
	cout << "n = " << n << endl;


	//int GaussianKernel[5][5];
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<5; j++)
		{
			cout << "please enter 5*5 GaussianKernel: GaussianKernel[" << i+1 << "][" << j+1 << "] = ";
			cin >> GaussianKernel[i][j];
		}
	}

	cout << "The current GaussianKernel as follows: " << endl;
	for(int j=0; j<5; j++)
		cout << GaussianKernel[0][j] << "  ";
	cout << endl;

	for(int j=0; j<5; j++)
		cout << GaussianKernel[1][j] << "  ";
	cout << endl;

	for(int j=0; j<5; j++)
		cout << GaussianKernel[2][j] << "  ";
	cout << endl;

	for(int j=0; j<5; j++)
		cout << GaussianKernel[3][j] << "  ";
	cout << endl;

	for(int j=0; j<5; j++)
		cout << GaussianKernel[4][j] << "  ";
	cout << endl;

	int Kernel_sum = 0, num;
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<5; j++)
		{
			Kernel_sum = Kernel_sum + GaussianKernel[i][j];
		}
	}
	cout << "Kernel_sum = " << Kernel_sum << endl;

	cout << "please enter 1 to continue perform the Sobel optation : ";
	cin >> num;
#endif

	if(num=1)
	{
		/// Load an image
		src = imread( argv[1] );


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
				clone_src_gray2.at<uchar>(y,x) = sum_gray;
			}
		}
		imshow("final", clone_src_gray2);
		waitKey(0);
	}
	return 0;
}

