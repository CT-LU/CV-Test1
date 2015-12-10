#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <signal.h>
#include <syslog.h>
#include <errno.h>

#include <iostream>
#include <ctype.h>
#include <math.h>

#include "gmmWithCuda.h"

using namespace cv;
using namespace std;

#define FRAME_WIDTH		1280
#define FRAME_HEIGHT		720

int main(int argc, char** argv)
{
	VideoCapture cap;
	Mat frame;

	frame.create(Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC1);

	//if ( frame.isContinuous() ) cout << "yes" << endl;
	//Open RGB Camera
	cap.open(0);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	if( !cap.isOpened() )
	{
		cout << "Can not open camera !!" << endl;
		return -1;
	}

	//read frame
	cap >> frame;
	if( frame.empty() )
	{
		cout << "Can not read data from the Camera !!" << endl;
		return -1;
	}

	gpu_initialize_gmm(frame.ptr(0));

	cout << "frame.cols: " << frame.cols << endl;
	cout << "frame.rows: " << frame.rows << endl;

	for(;;)
	{
		//Get RGB Image
		cap >> frame;

		if( frame.empty() )
		{
			cout << "Can not read data from the Camera !!" << endl;
			return -1;
		}
		
		//GMM output
		Mat gmm_frame;
		gmm_frame.create(frame.size(), frame.type());
		gmm_frame = Mat::zeros(frame.size(), CV_8UC1);
		
		gpu_perform_gmm(frame.ptr(0), gmm_frame.ptr(0));
		//Show the GMM result image
		imshow("GMM", gmm_frame);

		//User Key Input
		char c = waitKey(10);
		if (c == 27) break; // got ESC
	}
	
	gpu_free_gmm();

	return 0;
}

