#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "depth_image_generator.hpp"

#include <iostream>

using namespace cv;
using namespace std;


int getDepthCamera(VideoCapture* capture){

	cout << "Depth Device opening ..." << endl;

	capture->open( CAP_OPENNI2_ASUS );
	if( !capture->isOpened() )
		capture->open( CAP_OPENNI_ASUS );


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

	//Dilate it
	//Mat dilate_element(10, 10, CV_8U, Scalar(1));
	//dilate(*image, *image, dilate_element, Point(-1, -1), 3);

	adaptiveThreshold(*image, *image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

	//imshow( "threshold depth map", image );

	return 0;
}

int filterDepthImage(Mat *src_image, Mat *dst_image, int start_depth, int end_depth){
	
	//Joe add threshold
	threshold(*src_image, *dst_image, start_depth, 255, THRESH_TOZERO);
	threshold(*dst_image, *dst_image, end_depth, 255, THRESH_TOZERO_INV);
	
	//Dilate it
	//Mat dilate_element(10, 10, CV_8U, Scalar(1));
	//dilate(*dst_image, *dst_image, dilate_element, Point(-1, -1), 3);

	adaptiveThreshold(*dst_image, *dst_image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

	//imshow( "threshold depth map", image );

	return 0;
}

int setDepthImageROI(Mat *image, int x, int y, int width, int height){
	
	//print image property
	//cout << "image->rows: " << image->rows << ", image.cols: " << image->cols << endl;

	//create ROI
	Rect roi = Rect(x, y, width, height);
	
	//Draw ROI on Source imagea
	Mat new_image;
	(*image).copyTo(new_image);
	//cout << "new_image.rows: " << new_image.rows << ", new_image.cols: " << new_image.cols << endl;

	rectangle(new_image, roi, Scalar(181,186,10), 3, 8);

	/*
	circle(new_image, Point(220,180), 5, Scalar(181, 186, 10), -1, 8, 0);
	circle(new_image, Point(420,180), 5, Scalar(181, 186, 10), -1, 8, 0);
	circle(new_image, Point(220,300), 5, Scalar(181, 186, 10), -1, 8, 0);
	circle(new_image, Point(420,300), 5, Scalar(181, 186, 10), -1, 8, 0);
	*/

	imshow("Draw ROI", new_image);
	
	//set ROI on image
	//Mat roi_image(*image, roi);
	//imshow("ROI Depth Image", roi_image);

}
