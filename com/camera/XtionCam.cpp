#include "XtionCam.hpp"
#include <iostream>

using namespace cv;
using namespace std;

XtionCam::XtionCam() {
	cout << "Xtion Camera opening ..." << endl;

	capture.open( CAP_OPENNI2_ASUS );
	if( !capture.isOpened() )
		capture.open( CAP_OPENNI_ASUS );


	cout << "done." << endl;

	if( !capture.isOpened() )
	{
		cout << "Can not open a capture object." << endl;
		exit(1);
	}
}

XtionCam::~XtionCam() {
}

int XtionCam::getBgrImage(cv::Mat &bgr_image) {

	if( !capture.grab() )
	{
		cout << "Can not grab bgr images." << endl;
		return -1;
	}
	else
	{
		capture.retrieve( bgr_image, CAP_OPENNI_BGR_IMAGE );
	}

	return 0;
}

int XtionCam::getDepthImage(cv::Mat &depth_image) {
	
	if( !capture.grab() )
	{
		cout << "Can not grab images." << endl;
		return -1;
	}
	else
	{

		if( capture.retrieve( depth_image, CAP_OPENNI_DEPTH_MAP ) )
		{
			const float scaleFactor = 0.05f;
			depth_image.convertTo( depth_image, CV_8UC1, scaleFactor );
		}

	}

	return 0;
}	

int XtionCam::get2Image(cv::Mat &bgr_image, cv::Mat &depth_image) {

	if( !capture.grab() )
	{
		cout << "Can not grab images." << endl;
		return -1;
	}
	else
	{
        //get Depth
		if( capture.retrieve( depth_image, CAP_OPENNI_DEPTH_MAP ) )
		{
			const float scaleFactor = 0.05f;
			depth_image.convertTo( depth_image, CV_8UC1, scaleFactor );

		}
        //get BGR
		capture.retrieve( bgr_image, CAP_OPENNI_BGR_IMAGE );
	}

	return 0;
}	

