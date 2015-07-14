#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

int vmin = 10;
int vmax = 256;
int smin = 30;

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

void setUserInterface()
{
	namedWindow( "Histogram", 0 );
	namedWindow( "CamShift Demo", 0 );
	createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
	createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
	createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

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


static void filterDepthImage(const Mat& image, int start_depth, int end_depth){
	//Joe add threshold
	//start_depth = 30
	//end_depth = 45
	threshold(image, image, start_depth, 255, THRESH_TOZERO);
	threshold(image, image, end_depth, 255, THRESH_TOZERO_INV);

	//Need to filter x and y planes

	adaptiveThreshold(image, image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

	//imshow( "threshold depth map", image );


}

int getContours(Mat *image, vector< vector<Point> > *contours, vector<Vec4i> *hierarchy){

	//MorphologyEx
	Mat elementA(3, 3, CV_8U, Scalar(1));
	morphologyEx(*image, *image, cv::MORPH_CLOSE, elementA);

	//findContours
	//get Contours external
	findContours(*image, *contours, *hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	return 0;
}

int findLargestContourIndex(vector< vector<Point> > contours){

	//find largest contours
	int largest_area = 0;
	int largest_contour_index = -1;
	for( size_t i = 0; i< contours.size(); i++ ) // iterate through each contour.
	{
		double a = contourArea( contours[i], false);  //  Find the area of contour
		if(a > largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			//bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}
	}
	return largest_contour_index;
}

int drawContours(Mat *drawing, vector< vector<Point> > *contours, vector<Vec4i> *hierarchy, int thickness, int largest_contour_index, int area_threshold){

	//cout << "contours->size(): " << contours->size() << endl;
	//cout << "largest_contour_index: " << largest_contour_index << endl;
	//cout << "area_threshold: " << area_threshold << endl;
	
	for( size_t i = 0; i < contours->size(); i++ )
	{
		//cout << "For Loop: " << i << endl;
		//cout << "contours[i]: " << contourArea((*contours)[i]) << endl;

		if((contourArea((*contours)[i]) <= area_threshold) || i != largest_contour_index ){
			continue;
		}

		drawContours( *drawing, *contours, int(i), Scalar(255,0,0), thickness, 8, *hierarchy, 0, Point() );

	}
	//imshow("drawContours", drawing);
	
	return 0;
}


//********************************************************************************************************************

int main(int argc, char** argv)
{

	VideoCapture capture;
	Mat image, drawing;

	//histogram
	Mat hsv, hue, mask;
	Mat hist, histimg = Mat::zeros( 200, 320, CV_8UC3 );
	Mat backproj;
	bool change_object = true;
	Rect selection;

	//camshift
	Rect trackWindow;
	int hsize = 16;
	float hranges[] = { 0, 180};
	const float* phranges = hranges;

	//getDepthCamera
	if(getDepthCamera(&capture) == -1)
	{
		cout << "Open Depth Camera Failed !!" << endl;
		return -1;
	}else{
		cout << "Depth Camera Open Succeed !!" << endl;
	}

	//set UI
	setUserInterface();

	//condition
	int area_threshold = 1000;
	int area_tracking_object = 2000;

	for(;;)
	{
		//getDepthImage
		getDepthImage(capture, &image);

		//filterDepthImage
		filterDepthImage(image, (int)30, (int)45);

		//show depth image
		//imshow("Main Program for Depth Camera", image);

		//handle Contours   
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;

		//findContours  OK
		getContours(&image, &contours, &hierarchy);

		//get largest contour
		int largest_contour_index = findLargestContourIndex(contours);


		//cout << "largest_contour_index: " << largest_contour_index << endl;

		//draw Contours
		drawing = Mat::zeros(image.size(), CV_8UC3);
		drawContours(&drawing, &contours, &hierarchy, CV_FILLED, largest_contour_index, area_threshold);

		//imshow("drawContours", drawing);


		//Using CamShift tracking Object


		//CamShift
		//cout << "Area of largest contour: " << contourArea(contours[largest_contour_index]) << endl;

		if( largest_contour_index != -1 && contourArea(contours[largest_contour_index]) >= area_tracking_object){
			//Color space transform -> from BGR to HSV
			cvtColor(drawing, hsv, COLOR_BGR2HSV);

			int _vmin = vmin, _vmax = vmax;
			inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
					Scalar(180, 256, MAX(_vmin, _vmax)), mask);
			int ch[] = {0, 0};
			hue.create(hsv.size(), hsv.depth());
			mixChannels(&hsv, 1, &hue, 1, ch, 1);


			if(change_object){
				//change flag
				change_object = false;
				//get selected object
				selection = boundingRect(contours[largest_contour_index]);

				Mat roi(hue, selection), maskroi(mask, selection);
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
				normalize(hist, hist, 0, 255, NORM_MINMAX);

				trackWindow = selection;

				histimg = Scalar::all(0);
				int binW = histimg.cols / hsize;
				Mat buf(1, hsize, CV_8UC3);

				for( int i = 0; i < hsize; i++ )
				{
					buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
				}
				cvtColor(buf, buf, COLOR_HSV2BGR);

				for( int i = 0; i < hsize; i++ )
				{
					int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
					rectangle( histimg, Point(i*binW,histimg.rows),
							Point((i+1)*binW,histimg.rows - val),
							Scalar(buf.at<Vec3b>(i)), -1, 8 );
				}

			}

			//Update CamShift tracking object
			calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
			backproj &= mask;
			RotatedRect trackBox = CamShift(backproj, trackWindow,
					TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));

			//adaptive trackWindow
			if( trackWindow.area() <= 1 )
			{
				int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
				trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
						trackWindow.x + r, trackWindow.y + r) &
					Rect(0, 0, cols, rows);
			}

			//Draw focus ellipse
			ellipse( drawing, trackBox, Scalar(0,0,255), 3, LINE_AA );

			//Draw focus retangle
			rectangle( drawing, trackWindow, Scalar(0,255,0), 3, 8 );

			//Draw center of trackBox
			circle( drawing, trackBox.center, 5, Scalar(181,186,10), -1, 8, 0 );

		}else{
			change_object = true;
		}

		imshow("Histogram", histimg);
		imshow("CamShift tracking Object", drawing);


		waitKey(30);
	}

}





















