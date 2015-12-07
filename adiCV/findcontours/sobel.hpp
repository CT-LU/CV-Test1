#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;

//Sobel functions
//____________________________________________________________________________________________________________________________

int gMaskSum(double &G_center, double &pdf_0, double &pdf_1, double &pdf_2, const double pi, int gMask[5][5]);

void GaussianBlur(Mat &src_gray, Mat &src_GB, int Kernel_sum, int gMask[5][5], int image_height, int image_width); 

void Sobel(Mat &src_GB, Mat &src_Sobel, int image_height, int image_width);

