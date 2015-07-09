#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;


int getDepthCamera(VideoCapture* capture){

	cout << "Depth Device opening ..." << endl;

	capture->open( CAP_OPENNI2 );
	if( !capture->isOpened() )
		capture->open( CAP_OPENNI );


	cout << "done." << endl;

	if( !capture->isOpened() )
	{
		cout << "Can not open a capture object." << endl;
		return -1;
	}

	return 0;
}


int getDepthImage(VideoCapture capture, Mat *image){

	//depth image
	//Mat depthMap;

	if( !capture.grab() )
	{
		cout << "Can not grab images." << endl;
		return -1;
	}
	else
	{

		if( capture.retrieve( *image, CAP_OPENNI_DEPTH_MAP ) )
		{
			const float scaleFactor = 0.05f;
			//Mat show;
			image->convertTo( *image, CV_8UC1, scaleFactor );

			//imshow( "depth map", *image );

		}

	}

	return 0;
}

int filterDepthImage(Mat *image, int start_depth, int end_depth){
	//Joe add threshold
	//start_depth = 30
	//end_depth = 45
	threshold(*image, *image, start_depth, 255, THRESH_TOZERO);
	threshold(*image, *image, end_depth, 255, THRESH_TOZERO_INV);

	//Need to filter x and y planes
	//
	//

	adaptiveThreshold(*image, *image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

	//imshow( "threshold depth map", image );

	return 0;
}

