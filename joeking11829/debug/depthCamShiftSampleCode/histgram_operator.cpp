#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;



int calcHistData(Mat* image, Mat* mask, Mat* hist, int hsize, const float* phranges)
{
	//calculate Histogram
	calcHist(image, 1, 0, *mask, *hist, 1, &hsize, &phranges);

	return 0;
}


int normalizeHistData(Mat* image, int lowerBound, int upperBound, int normType)
{
	//Histogram normalization
	//Default -> lowerBound=0, upperBound=255, normType=NORM_MINMAX
	normalize(*hist, *hist, lowerBound, upperBound, normType);

	return 0;
}


int createHistView(Mat* hist, Mat* histimg, int hsize )
{
	cout << "histimg->cols: " << histimg->cols << "histimg->rows: " << histimg->rows << endl;

	*histimg = Scalar::all(0);
	int binW = histimg->cols / hsize;
	Mat buf(1, hsize, CV_8UC3);

	for( int i = 0; i < hsize; i++ )
	{
		buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
	}
	cvtColor(buf, buf, COLOR_HSV2BGR);

	for( int i = 0; i < hsize; i++ )
	{
		int val = saturate_cast<int>((*hist).at<float>(i)*(histimg->rows/255));
		rectangle( *histimg, Point(i*binW, histimg->rows),
				Point((i+1)*binW, histimg->rows - val),
				Scalar(buf.at<Vec3b>(i)), -1, 8 );
	}

	return 0;
}

