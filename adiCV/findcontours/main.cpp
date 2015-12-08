#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include "sobel.hpp"
#include "findcontours.hpp"

#define matric(x, y) src_GB_raw_data[(y)*image_width+(x)]

using namespace std;
using namespace cv;

int main( int argc, char** argv )
{

	Mat src; //Load image
	Mat src_gray; //Gray image
	Mat src_GB; //Used in implementing GaussianBlur 
	Mat src_Sobel; //Used in implementing Sobel Operation
	int **examined;
	int **pixelNBD;


	/*
	 * =======================================================================================
	 *         Sobel Operation 
	 * =======================================================================================
	 */

	//Set Gaussian distribution parms
	int gMask[5][5];
	double pdf_0, pdf_1, pdf_2, pi = 3.14159;
	double g_center;

	//Set findcontours parms
	vector<Point> vector_point;
	vector<vector<Point> > contours_implement;

	// Compute gMask and sum of each element of gMask 
	g_center = atoi(argv[2]); // G_center is the center value of the gMask
	int Kernel_sum = gMaskSum(g_center, pdf_0, pdf_1, pdf_2, pi, gMask);

	// Load an image
	src = imread( argv[1] );
	imshow("Image", src);

	//Get the width and height of image
	int image_width, image_height;
	image_width = src.cols;
	image_height = src.rows;

	examined = new int*[image_height];
	for(int i = 0; i < image_height; i++){
		examined[i] = new int[image_width];
	}

	pixelNBD = new int*[image_height];
	for(int i = 0; i < image_height; i++){
		pixelNBD[i] = new int[image_width];
	}


	//RGB image => GRAY image
	cvtColor( src, src_gray, CV_RGB2GRAY );

	//Implement GaussianBlur
	src_GB = src_gray.clone();
	uchar* src_GB_raw_data = src_GB.ptr(0);
	GaussianBlur(src_gray, src_GB, Kernel_sum, gMask, image_height, image_width);
	imshow("GaussianBlur", src_GB);

	//Implenent the Sobel 
	src_Sobel = src_GB.clone();
	uchar* src_Sobel_raw_data = src_Sobel.ptr(0);
	Sobel(src_GB, src_Sobel, image_height, image_width,pixelNBD);
	imshow("Sobel image (binary)", src_Sobel);



	/*
	 * =======================================================================================
	 *         Topology Structural Analysis 
	 * =======================================================================================
	 */
	int comNBD = 1;
	int x1, y1, x2, y2, x3, y3, x4, y4; 

	//set initially number 
	/*	for(int y = 0; y < image_height; y++){
		for(int x = 0; x < image_width; x++){
		examined[y][x] = 0; 
		if(src_Sobel_raw_data[y*image_width+x] == 255){ 
		pixelNBD[y][x] = 1; 
		}
		else{
		pixelNBD[y][x] = 0;
		}
		}
		}*/

	//tv raster scan
	for(int y = 1; y < image_height - 1; y++){
		for(int x = 1; x < image_width - 1; x++){
			// 1(a)
			if((pixelNBD[y][x] == 1) && (pixelNBD[y][x-1] == 0) ){
				comNBD = comNBD + 1;
				x2 = x - 1;
				y2 = y;

				// 3.1 case 0.5 => pixel_(x-1, y-1) = 1
				if(pixelNBD[y-1][x-1] != 0){
					cout << "check 3" << endl;
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 

						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){		
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value
					}
					contours_implement.push_back(vector_point);
					vector_point.clear();
				}


				// 3.1 case 1 => pixel_(x, y-1) = 1

				else if(pixelNBD[y-1][x] != 0){
					cout << "check 4" << endl;
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 
						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){		
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value

					}
					contours_implement.push_back(vector_point);
					vector_point.clear();
				}

				// 3.1 case 1.5 => pixel_(x+1, y-1) = 1

				else if(pixelNBD[y-1][x+1] != 0){
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 
						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){		
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value
					}
					contours_implement.push_back(vector_point);
					vector_point.clear();
				}


				// 3.1 case 2 => pixel_(x+1,y) = 1

				else if(pixelNBD[y][x+1] != 0){ 
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 
						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value
					}
					contours_implement.push_back(vector_point);
					vector_point.clear();

				}

				// 3.1 case 2.5 => pixel_(x+1,y+1) = 1

				else if(pixelNBD[y+1][x+1] != 0){ 
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 
						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value
					}
					contours_implement.push_back(vector_point);
					vector_point.clear();

				}

				// 3.1 case 3 => pixel_(x,y+1) = 1

				else if(pixelNBD[y+1][x] != 0){
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 
						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value
					}
					contours_implement.push_back(vector_point);
					vector_point.clear();

				}

				// 3.1 case 3.5 => pixel_(x-1,y+1) = 1

				else if(pixelNBD[y+1][x-1] != 0){
					function3_1(x, y, x1, y1); // 3.1
					function3_2(x, y, x1, y1, x2, y2, x3, y3, examined, image_height, image_width); // 3.2

					// 3.3
					while(1){ 
						find_point(x2, y2, x3, y3, x4, y4, pixelNBD, examined);
						function3_4(x3, y3, comNBD, pixelNBD, examined, vector_point, image_height, image_width); // 3.4 (a)(b) 

						// 3.5
						if((x4 == x) && (y4 == y) || (x3 == x1) && (y3 == y1) || (x3 == x4) && (y3 == y4)){
							break;
						}
						function3_5(x2, y2, x3, y3, x4, y4); // 3.5 change point value
					}
					contours_implement.push_back(vector_point);
					vector_point.clear();

				}
				else if(pixelNBD[y-1][x] != 1 && pixelNBD[y][x+1] != 1 && pixelNBD[y+1][x] != 1){
					pixelNBD[y][x] = -comNBD;
					vector_point.push_back(Point(x3,y3));
					contours_implement.push_back(vector_point);
					vector_point.clear();
				}
			}
		}
	}

	RNG rng(12345);
	vector<Vec4i> hierarchy;


	Mat drawing_implement = Mat::zeros( src_Sobel.size(), CV_8UC3 );

	for( int i = 0; i< contours_implement.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing_implement, contours_implement, i, color, 2, 8, 0, 0, Point() );
	}

	imshow( "Contours_implement", drawing_implement );

	vector<vector<Point> > contours_origin;
	findContours( src_Sobel, contours_origin, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
	Mat drawing_origin = Mat::zeros( src_Sobel.size(), CV_8UC3 );

	for( int i = 0; i< contours_origin.size(); i++ )
	{
		Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
		drawContours( drawing_origin, contours_origin, i, color, 2, 8, 0, 0, Point() );
	}

	imshow( "Contours_origin", drawing_origin );

	waitKey(0);
	delete2dArray(pixelNBD, image_height, image_width);
	delete2dArray(examined, image_height, image_width);
	return 0;
}





