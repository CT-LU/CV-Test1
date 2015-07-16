#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;


int getDepthCamera(VideoCapture* capture);

int getDepthImage(VideoCapture capture, Mat *image);

int filterDepthImage(Mat *image, int start_depth, int end_depth);

int filterDepthImage(Mat *src_image, Mat *dst_image, int start_depth, int end_depth);

int setDepthImageROI(Mat *image, int x, int y, int width, int height);


