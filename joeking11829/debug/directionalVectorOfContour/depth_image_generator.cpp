#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "depth_image_generator.hpp"

#include <iostream>

using namespace cv;
using namespace std;

static void colorizeDisparity( const Mat& gray, Mat& rgb, double maxDisp=-1.f, float S=1.f, float V=1.f )
{
    CV_Assert( !gray.empty() );
    CV_Assert( gray.type() == CV_8UC1 );

    if( maxDisp <= 0 )
    {
        maxDisp = 0;
        minMaxLoc( gray, 0, &maxDisp );
    }

    rgb.create( gray.size(), CV_8UC3 );
    rgb = Scalar::all(0);
    if( maxDisp < 1 )
        return;

    for( int y = 0; y < gray.rows; y++ )
    {
        for( int x = 0; x < gray.cols; x++ )
        {
            uchar d = gray.at<uchar>(y,x);
            unsigned int H = ((uchar)maxDisp - d) * 240 / (uchar)maxDisp;

            unsigned int hi = (H/60) % 6;
            float f = H/60.f - H/60;
            float p = V * (1 - S);
            float q = V * (1 - f * S);
            float t = V * (1 - (1 - f) * S);

            Point3f res;

            if( hi == 0 ) //R = V,  G = t,  B = p
                res = Point3f( p, t, V );
            if( hi == 1 ) // R = q, G = V,  B = p
                res = Point3f( p, V, q );
            if( hi == 2 ) // R = p, G = V,  B = t
                res = Point3f( t, V, p );
            if( hi == 3 ) // R = p, G = q,  B = V
                res = Point3f( V, q, p );
            if( hi == 4 ) // R = t, G = p,  B = V
                res = Point3f( V, p, t );
            if( hi == 5 ) // R = V, G = p,  B = q
                res = Point3f( q, p, V );

            uchar b = (uchar)(std::max(0.f, std::min (res.x, 1.f)) * 255.f);
            uchar g = (uchar)(std::max(0.f, std::min (res.y, 1.f)) * 255.f);
            uchar r = (uchar)(std::max(0.f, std::min (res.z, 1.f)) * 255.f);

            rgb.at<Point3_<uchar> >(y,x) = Point3_<uchar>(b, g, r);
        }
    }
}

static float getMaxDisparity( VideoCapture& capture )
{
    const int minDistance = 400; // mm
    float b = (float)capture.get( CAP_OPENNI_DEPTH_GENERATOR_BASELINE ); // mm
    float F = (float)capture.get( CAP_OPENNI_DEPTH_GENERATOR_FOCAL_LENGTH ); // pixels
    return b * F / minDistance;
}


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

int getDepthImage(VideoCapture capture, Mat *gray_image, Mat *color_image){

	//depth image
	//Mat depthMap;

	if( !capture.grab() )
	{
		cout << "Can not grab images." << endl;
		return -1;
	}
	else
	{

		if( capture.retrieve( *gray_image, CAP_OPENNI_DEPTH_MAP ) )
		{
			const float scaleFactor = 0.05f;
			//Mat show;
			gray_image->convertTo( *gray_image, CV_8UC1, scaleFactor );

			//imshow( "depth map", *image );

		}

		//get Color depth image
		Mat disparityMap;
		if( capture.retrieve( disparityMap, CAP_OPENNI_DISPARITY_MAP ) )
		{
			Mat colorDisparityMap;
			colorizeDisparity( disparityMap, colorDisparityMap, -1 );
			//Mat validColorDisparityMap;
			colorDisparityMap.copyTo( *color_image, disparityMap != 0 );
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

int filterDepthImage(Mat *src_image, Mat *gray_dst_image, Mat *dst_image, int start_depth, int end_depth){

	//Joe add threshold
	threshold(*src_image, *gray_dst_image, start_depth, 255, THRESH_TOZERO);
	threshold(*gray_dst_image, *gray_dst_image, end_depth, 255, THRESH_TOZERO_INV);

	//Dilate it
	//Mat dilate_element(10, 10, CV_8U, Scalar(1));
	//dilate(*dst_image, *dst_image, dilate_element, Point(-1, -1), 3);

	adaptiveThreshold(*gray_dst_image, *dst_image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

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
