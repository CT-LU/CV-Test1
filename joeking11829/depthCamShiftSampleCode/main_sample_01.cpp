#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "depth_image_generator.hpp"
#include "contours_operator.hpp"


#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

int vmin = 10;
int vmax = 256;
int smin = 30;

void setUserInterface()
{
	namedWindow( "Histogram", 0 );
	namedWindow( "CamShift Demo", 0 );
	createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
	createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
	createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

}

int main(int argc, char** argv){

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
		filterDepthImage(&image, (int)30, (int)45);

		//show depth image
		//imshow("Main Program for Depth Camera", image);

		//handle Contours
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		//findContours
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

		}else{
			change_object = true;
		}
		
		imshow("Histogram", histimg);
		imshow("CamShift tracking Object", drawing);
		

		waitKey(30);
	}

}

