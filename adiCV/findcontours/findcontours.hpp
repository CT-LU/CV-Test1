#if 0
#ifndef __FINDCONTOURS_H__
#define __FINDCONTOURS_H__

void function3_1(int &x, int &y, int &x1, int &y1);

void function3_2(int x, int y, int &x1, int &y1, int &x2, int &y2, int &x3, int &y3, int examined[1000][1000]);

void find_point(int &x2, int &y2, int &x3, int &y3, int &x4, int &y4, int pixelNBD[1000][1000], int examined[1000][1000]);

void function3_4(int &x3, int &y3, int &comNBD, int pixelNBD[1000][1000], int examined[1000][1000], std::vector<Point> &vector_point);

void function3_5(int &x2, int &y2, int &x3, int &y3, int &x4, int &y4);

#endif
#endif

#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;

//Findcontours functions
//____________________________________________________________________________________________________________________________

void function3_1(int &x, int &y, int &x1, int &y1);

void function3_2(int x, int y, int &x1, int &y1, int &x2, int &y2, int &x3, int &y3, int **examined, int image_height, int image_width);

void function3_4(int &x3, int &y3, int &comNBD, int **pixelNBD, int **examined, vector<Point> &vector_point, int image_height, int image_width);

void function3_5(int &x2, int &y2, int &x3, int &y3, int &x4, int &y4);

int findNewCenter(int &x3, int &y3, int &x4, int &y4, int **pixelNBD, int **examined, int x_shift, int y_shift, int image_height, int image_width);

void find_point(int &x2, int &y2, int &x3, int &y3, int &x4, int &y4, int **pixelNBD, int **examined);

//____________________________________________________________________________________________________________________________


//Delete2dArray
void delete2dArray(int **pixelNBD, int image_height, int image_width); 
