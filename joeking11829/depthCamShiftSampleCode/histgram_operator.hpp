#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;


int calcHistData(Mat* image, Mat* mask, Mat* hist, int hsize, const float* phranges);

int normalizeHistData(Mat* image, int lowerBound, int upperBound, int normType);

int createHistView(Mat* hist, Mat* histimg, int hsize );

