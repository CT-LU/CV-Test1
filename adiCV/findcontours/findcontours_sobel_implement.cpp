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
	//imshow("Not GaussianBlur image", src_gray);
	//imshow("GaussianBlur image", clone_src_gray1);


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
			if(sum_gray > 100){
				sum_gray = 255;			
			}else{
				sum_gray = 0;				
			}
			clone_src_gray2.at<uchar>(y,x) = sum_gray;
		}
	}
	imshow("Sobel image (binary)", clone_src_gray2);
	//cout << "clone_src_gray2.at<uchar>(200,200) = " << clone_src_gray1.at<uchar>(200,200) << endl;

	/*
	 * =======================================================================================
	 *         Topology Structural Analysis 
	 * =======================================================================================
	 */

	//ulong iii = 0;
	int comNBD = 1;
	int pixelNBD[512][512];
	int examined[512][512];
	int x1, y1, x2, y2, x3, y3, x4, y4; 

	//int tmp1 = 0, tmp2 = 0;

	//set initially number  
	for(int y = 0; y < clone_src_gray2.rows; y++){
		for(int x = 0; x < clone_src_gray2.cols; x++){
			examined[x][y] = 0;
			if(clone_src_gray2.at<uchar>(y,x) == 255){
				pixelNBD[x][y] = 1;
				//tmp1 = tmp1 + 1;
			}
			else{
				pixelNBD[x][y] = 0;
				//tmp2 = tmp2 + 1;
			}

		}
	}
	//cout << "tmp1 = " << tmp1 << endl;
	//cout << "tmp2 = " << tmp2 << endl; 

	//for(int y = 3; y < clone_src_gray2.rows - 3; y++){
	//	for(int x = 3; x < clone_src_gray2.cols - 3; x++){
	//for(int y = 4; y < 5; y++){
	//	for(int x = 344; x < 345; x++){
	for(int y = 2; y < 3; y++){
		for(int x = 1; x < 2; x++){
	//for(int y = 100; y < 101; y++){
	//	for(int x = 100; x < 300; x++){
			// 1(a)
			if((pixelNBD[x][y] == 1) && (pixelNBD[x-1][y] == 0) ){

				comNBD = comNBD + 1;
				x2 = x - 1;
				y2 = y;
				examined[x][y] = 1;
				examined[x-1][y] = 1;	

				// 3.1 case 1 => pixel_(x, y-1) = 1

				if(pixelNBD[x][y-1] == 1){

					x1 = x;
					y1 = y - 1;

					// 3.2
					x2 = x1;
					y2 = y1;
					x3 = x;
					y3 = y;
					examined[x][y-1] = 1;

					// 3.3
					while(1){ 
						if((x2-x3>0) && (y2 == y3)){
							if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 < y3)){
							if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
						}else if((x2 < x3) && (y2 == y3)){
							if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 > y3)){
							if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
						}	

						// 3.4 (a)(b) 
						if(pixelNBD[x3+1][y3] == 0 && examined[x3+1][y3] == 1){
							pixelNBD[x3][y3] = -comNBD;
						}else if(pixelNBD[x3+1][y3] != 0 && examined[x3+1][y3] == 1 && pixelNBD[x3][y3] == 1){
							pixelNBD[x3][y3] = comNBD;
						}

						// 3.5
						cout << "x4 = " << x4 << endl;
						cout << "y4 = " << y4 << endl;
						if(/*(x4 == x) && (y4 == y) */  (x3 == x1) && (y3 == y1)){		
							break;
						}
						x2 = x3;
						y2 = y3;
						x3 = x4;
						y3 = y4;

					}
				}
#if 0
				else if(pixelNBD[x+1][y] == 1){ // 3.1 case 2 => pixel_(x+1,y) = 1
					x1 = x + 1;
					y1 = y;

					cout << "x = " << x << endl;
					cout << "y = " << y << endl;
					cout << "**************************" << endl; 
					cout << "x1 = " << x1 << endl;
					cout << "y1 = " << y1 << endl;
					cout << "**************************" << endl;
					
					// 3.2
					x2 = x1;
					y2 = y1;
					x3 = x;
					y3 = y;
					examined[x+1][y] = 1;

					// 3.3
					while(1){ 
						if((x2-x3>0) && (y2 == y3)){
							if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 < y3)){
							if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
						}else if((x2 < x3) && (y2 == y3)){
							if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 > y3)){
							if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
						}

						// 3.4 (a)(b) 
						if(pixelNBD[x3+1][y] == 0 && examined[x3+1][y3] == 1){
							pixelNBD[x3][y3] = -comNBD;
						}else if(pixelNBD[x3+1][y3] != 0 && examined[x3+1][y3] == 1 && pixelNBD[x3][y3] == 1){
							pixelNBD[x3][y3] = comNBD;
						}
						// 3.5
						cout << "x4 = " << x4 << endl;
						cout << "y4 = " << y4 << endl;
						cout << "--------------" << endl;
						cout << "x3 = " << x3 << endl;
						cout << "y3 = " << y3 << endl;
						cout << "--------------" << endl;
						if(/*(x4 == x) && (y4 == y)  && */ (x3 == x1) && (y3 == y1)){
							break;
						}
						x2 = x3;
						y2 = y3;
						x3 = x4;
						y3 = y4;

					}

				}


				// 3.1 case 3 => pixel_(x,y+1) = 1
				else if(pixelNBD[x][y+1] == 1){
					x1 = x;
					y1 = y + 1;

					// 3.2
					x2 = x1;
					y2 = y1;
					x3 = x;
					y3 = y;
					examined[x][y+1] = 1;

					// 3.3
					while(1){ 
						if((x2-x3>0) && (y2 == y3)){
							if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 < y3)){
							if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
						}else if((x2 < x3) && (y2 == y3)){
							if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 > y3)){
							if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
						}	
					}

					// 3.4 (a)(b) 
					if(pixelNBD[x3+1][y3] == 0 && examined[x3+1][y3] == 1){
						pixelNBD[x3][y3] = -comNBD;
					}else if(pixelNBD[x3+1][y3] != 0 && examined[x3+1][y3] == 1 && pixelNBD[x3][y3] == 1){
						pixelNBD[x3][y3] = comNBD;
					}
					// 3.5
					cout << "x4 = " << x4 << endl;
					cout << "y4 = " << y4 << endl;
					cout << "--------------" << endl;
					cout << "x3 = " << x3 << endl;
					cout << "y3 = " << y3 << endl;
					cout << "--------------" << endl;
					if(/*(x4 == x) && (y4 == y)  && */ (x3 == x1) && (y3 == y1)){
						break;
					}
					x2 = x3;
					y2 = y3;
					x3 = x4;
					y3 = y4;

				}

				else if(pixelNBD[x][y-1] != 1 && pixelNBD[x+1][y] != 1 && pixelNBD[x][y+1] != 1){
					pixelNBD[x][y] = -comNBD;
				}
#endif

			}

#if 0
			// 1(b)
			else if(pixelNBD[x][y] >= 1 && pixelNBD[x+1][y] == 0){
				comNBD = comNBD + 1;
				x2 = x + 1;
				y2 = y;
				examined[x][y] = 1;
				examined[x+1][y] = 1;	

				// 3.1 case 1 => pixel_(x, y+1) = 1
				if(pixelNBD[x][y+1] == 1){
					x1 = x;
					y1 = y + 1;

					// 3.2
					x2 = x1;
					y2 = y1;
					x3 = x;
					y3 = y;
					examined[x][y+1] = 1;

					// 3.3 
					while(1){ 
						if((x2-x3>0) && (y2 == y3)){
							if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 < y3)){
							if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
						}else if((x2 < x3) && (y2 == y3)){
							if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 > y3)){
							if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
						}	
					}

					// 3.4 (a)(b) 
					if(pixelNBD[x3+1][y3] == 0 && examined[x3+1][y3] == 1){
						pixelNBD[x3][y3] = -comNBD;
					}else if(pixelNBD[x3+1][y3] != 0 && examined[x3+1][y3] == 1 && pixelNBD[x3][y3] == 1){
						pixelNBD[x3][y3] = comNBD;
					}
					// 3.5
					cout << "x4 = " << x4 << endl;
					cout << "y4 = " << y4 << endl;
					cout << "--------------" << endl;
					cout << "x3 = " << x3 << endl;
					cout << "y3 = " << y3 << endl;
					cout << "--------------" << endl;
					if(/*(x4 == x) && (y4 == y)  && */ (x3 == x1) && (y3 == y1)){
						break;
					}
					x2 = x3;
					y2 = y3;
					x3 = x4;
					y3 = y4;
				}



				// 3.1 case 2 => pixel_(x-1,y) = 1
				else if(pixelNBD[x-1][y] == 1){
					x1 = x - 1;
					y1 = y;

					// 3.2
					x2 = x1;
					y2 = y1;
					x3 = x;
					y3 = y;
					examined[x-1][y] = 1;

					// 3.3
					while(1){ 
						if((x2-x3>0) && (y2 == y3)){
							if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 < y3)){
							if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
						}else if((x2 < x3) && (y2 == y3)){
							if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 > y3)){
							if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
						}	
					}

					// 3.4 (a)(b) 
					if(pixelNBD[x3+1][y3] == 0 && examined[x3+1][y3] == 1){
						pixelNBD[x3][y3] = -comNBD;
					}else if(pixelNBD[x3+1][y3] != 0 && examined[x3+1][y3] == 1 && pixelNBD[x3][y3] == 1){
						pixelNBD[x3][y3] = comNBD;
					}
					// 3.5
					cout << "x4 = " << x4 << endl;
					cout << "y4 = " << y4 << endl;
					cout << "--------------" << endl;
					cout << "x3 = " << x3 << endl;
					cout << "y3 = " << y3 << endl;
					cout << "--------------" << endl;
					if(/*(x4 == x) && (y4 == y)  && */ (x3 == x1) && (y3 == y1)){
						break;
					}
					x2 = x3;
					y2 = y3;
					x3 = x4;
					y3 = y4;

				}


				// 3.1 case 3 => pixel_(x,y-1) = 1
				else if(pixelNBD[x][y-1] == 1){
					x1 = x;
					y1 = y - 1;

					// 3.2
					x2 = x1;
					y2 = y1;
					x3 = x;
					y3 = y;
					examined[x][y-1] = 1;

					// 3.3
					while(1){ 
						if((x2-x3>0) && (y2 == y3)){
							if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 < y3)){
							if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
						}else if((x2 < x3) && (y2 == y3)){
							if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
							else if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
						}else if((x2 == x3) && (y2 > y3)){
							if(pixelNBD[x3+1][y3] == 1){
								x4 = x3+1;
								y4 = y3;
								examined[x3+1][y3] = 1;
							}
							else if(pixelNBD[x3][y3-1] == 1){
								x4 = x3;
								y4 = y3-1;
								examined[x3][y3-1] = 1;
							}
							else if(pixelNBD[x3-1][y3] == 1){
								x4 = x3-1;
								y4 = y3;
								examined[x3-1][y3] = 1;
							}
							else if(pixelNBD[x3][y3+1] == 1){
								x4 = x3;
								y4 = y3+1;
								examined[x3][y3+1] = 1;
							}
						}	
					}
				}

				// 3.4 (a)(b) 
				if(pixelNBD[x3+1][y3] == 0 && examined[x3+1][y3] == 1){
					pixelNBD[x3][y3] = -comNBD;
				}else if(pixelNBD[x3+1][y3] != 0 && examined[x3+1][y3] == 1 && pixelNBD[x3][y3] == 1){
					pixelNBD[x3][y3] = comNBD;
				}
				// 3.5
				cout << "x4 = " << x4 << endl;
				cout << "y4 = " << y4 << endl;
				cout << "--------------" << endl;
				cout << "x3 = " << x3 << endl;
				cout << "y3 = " << y3 << endl;
				cout << "--------------" << endl;
				if(/*(x4 == x) && (y4 == y)  && */ (x3 == x1) && (y3 == y1)){
					break;
				}
				x2 = x3;
				y2 = y3;
				x3 = x4;
				y3 = y4;

			}


			else if(pixelNBD[x][y+1] != 1 && pixelNBD[x-1][y] != 1 && pixelNBD[x][y-1] != 1){
				pixelNBD[x][y] = -comNBD;
			}
#endif



		}
	}


	vector<Point> vector_point;
	vector<vector<Point> > contours_implement;
	int num=0;


	for(int y = 1; y < clone_src_gray2.rows - 1; y++){
		for(int x = 1; x < clone_src_gray2.cols - 1; x++){
			if(pixelNBD[x][y] == 2 || pixelNBD[x][y] == -2){
				num = num+1;
				cout << "num = " << num << endl;
				vector_point.push_back(Point(x,y));
			}
		}
	}

	contours_implement.push_back(vector_point);

	for(int i= 0; i < contours_implement.size(); i++)
	{
		for(int j= 0; j < contours_implement[i].size();j++) // run until j < contours[i].size();
		{
			cout << "contours_implement = " << contours_implement[i][j] << endl; //do whatever
		}
	}

	RNG rng(12345);
	vector<Vec4i> hierarchy;


	Mat drawing_implement = Mat::zeros( clone_src_gray2.size(), CV_8UC3 );
	for( int i = 0; i< contours_implement.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing_implement, contours_implement, i, color, 2, 8, 0, 0, Point() );
	}

	imshow( "Contours_implement", drawing_implement );

	vector<vector<Point> > contours_origin;
	findContours( clone_src_gray2, contours_origin, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
	Mat drawing_origin = Mat::zeros( clone_src_gray2.size(), CV_8UC3 );

	for( int i = 0; i< contours_origin.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing_origin, contours_origin, i, color, 2, 8, 0, 0, Point() );
	}

	for(int i= 0; i < contours_origin.size(); i++)
	{
		for(int j= 0; j < contours_origin[i].size();j++) // run until j < contours[i].size();
		{
			cout << "contours_origin = " << contours_origin[i][j] << endl; //do whatever
		}
	}

	imshow( "Contours_origin", drawing_origin );

	waitKey(0);

	return 0;
}

