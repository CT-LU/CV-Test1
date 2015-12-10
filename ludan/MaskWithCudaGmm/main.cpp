#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <iostream>

#include "MaskWithCudaGmm.hpp"

/*
 * this is an example to show how to use the class MaskWithCudaGmm
 */
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

	MaskWithCudaGmm masker(frame.ptr(0));

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
		
		Mat mask_frame = Mat::zeros(frame.size(), CV_8UC3);
		
		masker.maskWithCudaGmm(frame, mask_frame);	
		imshow("Mask Frame", mask_frame);

		//User Key Input
		char c = waitKey(10);
		if (c == 27) break; // got ESC
	}
	
	return 0;
}

