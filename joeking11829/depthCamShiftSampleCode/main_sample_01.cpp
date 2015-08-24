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

	//track offset of object
	Point resetPoint = Point(-1,-1);
	Point origin = resetPoint;
	Point current = resetPoint;
	
	//Calculate ROI parameter
	int source_image_width = 640;
	int source_image_height = 480;

	//ROI Rect
	int roi_rect_width = source_image_width * 0.6;
	int roi_rect_height = source_image_height * 0.5;
	int roi_rect_start_x = (source_image_width - roi_rect_width) * 0.5 ;
	int roi_rect_start_y = (source_image_height - roi_rect_height) * 0.5;

	//ROI to Right
	int roi_to_right_width = roi_rect_height * 0.32;
	int roi_to_right_height = roi_rect_height * 1.0;
	int roi_to_right_start_x = roi_rect_start_x;
	int roi_to_right_start_y = roi_rect_start_y + ((roi_rect_height - roi_to_right_height) * 0.5);
	
	
	//ROI to Left
	int roi_to_left_start_x = roi_rect_start_x + roi_rect_width - roi_to_right_width;
	int roi_to_left_start_y = roi_to_right_start_y;
	int roi_to_left_width = roi_to_right_width;
	int roi_to_left_height = roi_to_right_height;

	//ROI to Down
	int roi_to_down_start_x = roi_rect_start_x + (roi_rect_width * 0.25);
	int roi_to_down_start_y = roi_rect_start_y;
	int roi_to_down_width = roi_rect_width * 0.5;
	int roi_to_down_height = roi_rect_width * 0.25;

	//ROI to Up
	int roi_to_up_start_x = roi_to_down_start_x;
	int roi_to_up_start_y = roi_to_down_start_y + roi_rect_height - roi_to_down_height;
	int roi_to_up_width = roi_to_down_width;
	int roi_to_up_height = roi_to_down_height;

	
	

	//ROI for tracking
	Rect roi_rect(roi_rect_start_x, roi_rect_start_y, roi_rect_width, roi_rect_height);
	//ROI for Command
	//Right and Left
	Rect roi_to_right(roi_to_right_start_x, roi_to_right_start_y, roi_to_right_width, roi_to_right_height);
	Rect roi_to_left(roi_to_left_start_x, roi_to_left_start_y, roi_to_left_width, roi_to_left_height);
	//Up and Down
	Rect roi_to_up(roi_to_up_start_x, roi_to_up_start_y, roi_to_up_width, roi_to_up_height);
	Rect roi_to_down(roi_to_down_start_x, roi_to_down_start_y, roi_to_down_width, roi_to_down_height);

	//record displacement
	int x_displacement = -1;
	int y_displacement = -1;
	int threshold_of_displacement = 100;

	//command tag
	int gesture_command = -1;

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
	int area_threshold = 10;
	int area_tracking_object = 250;


	for(;;)
	{
		//getDepthImage
		getDepthImage(capture, &image);

		//filterDepthImage
		filterDepthImage(&image, (int)10, (int)40);

		//show depth image
		//imshow("Main Program for Depth Camera", image);

		//handle ROI
		setDepthImageROI(&image, roi_rect_start_x, roi_rect_start_y, roi_rect_width, roi_rect_height);

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
		

		//Dilate
		Mat dilate_element(10, 10, CV_8U, Scalar(1));
        	dilate(drawing, drawing, dilate_element, Point(-1, -1), 3);
		imshow("Origin drawContours", drawing);

		//Using CamShift tracking Object
		//CamShift
		//cout << "Area of largest contour: " << contourArea(contours[largest_contour_index]) << endl;

		if( largest_contour_index != -1 && contourArea(contours[largest_contour_index]) >= area_threshold ){
			//Color space transform -> from BGR to HSV	
			cvtColor(drawing, hsv, COLOR_BGR2HSV);

			int _vmin = vmin, _vmax = vmax;

			inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
					Scalar(180, 256, MAX(_vmin, _vmax)), mask);
			int ch[] = {0, 0};
			hue.create(hsv.size(), hsv.depth());
			mixChannels(&hsv, 1, &hue, 1, ch, 1);

			if(change_object){
				
				//if area of contour is too small then using next frame
				if(contourArea(contours[largest_contour_index]) <= area_tracking_object )
				{
					continue;
				}

				//reset histogram input for CamShift
				//change flag
				change_object = false;

				//reset origin point
				origin = resetPoint;
				//reset gesture_command
				gesture_command = -1;
				//reset displacement
				x_displacement = -1;
				y_displacement = -1;

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

			//Draw ROI
			//rectangle( drawing, roi_rect, Scalar(181,186,10), 3, 8 );

			//control offset
			if(roi_rect.contains(trackBox.center))
			{
				current = trackBox.center;
			}
			if(origin == resetPoint && roi_rect.contains(trackBox.center) )
			{
				//set new origin point
				origin = trackBox.center;
				cout << "origin.x: " << origin.x << " origin.y: " << origin.y << endl;

				//find what is the command
				if(roi_to_left.contains(origin))
				{
					//Left
					gesture_command = 1;

				}else if(roi_to_right.contains(origin))
				{
					//Right
					gesture_command = 2;
				}else if(roi_to_up.contains(origin))
				{
					//Up
					gesture_command = 3;
				}else if(roi_to_down.contains(origin))
				{
					//Down
					gesture_command = 4;
				}
			}


		}else{
			//reset selection
			change_object = true;

			if(current != resetPoint && origin != resetPoint)
			{
				cout << "current.x: " << current.x << " current.y: " << current.y << endl;
				//cout << "X offset -> current.x - origin.x: " << (current.x - origin.x) << endl;
				//cout << "Y offset -> current.y - origin.y: " << (current.y - origin.y) << endl;

				//count displacement
				x_displacement = abs(current.x - origin.x);
				y_displacement = abs(current.y - origin.y);
				cout << "x_displacement -> " << x_displacement << endl;
				cout << "y_displacement -> " << y_displacement << endl;

				//trigger xdotool command
				//judgment left right up and down


				/*
				//Method 0
				//for X
				if( (current.x - origin.x) > 0 )
				{
				//trigger Right Command
				cout << "Trigger LEFT LEFT LEFT LEFT !!!!" << endl;
				}else if( (current.x - origin.x) < 0 )
				{
				//trigger Left Command
				cout << "Trigger RIGHT RIGHT RIGHT RIGHT !!!!" << endl;
				}else if( (current.y - origin.y) < 0 )
				{
				//Trigger Up Command
				cout << "Trigger UP UP UP UP !!!!" << endl;
				}else if( (current.y - origin.y) > 0 )
				{
				//Trigger Down Command
				cout << "Trigger DOWN DOWN DOWN DOWN !!!!" << endl;
				}
				 */

				//Method 1
				switch(gesture_command){
					case 1:
						//Trigger Left Command
						if(x_displacement >= threshold_of_displacement)
						{
							cout << "Trigger LEFT LEFT LEFT LEFT !!!!" << endl;
						}
						break;
					case 2:
						//Trigger Right Command
						if(x_displacement >= threshold_of_displacement)
						{
							cout << "Trigger RIGHT RIGHT RIGHT RIGHT !!!!" << endl;
						}
						break;
					case 3:	
						//Trigger Up Command
						if(y_displacement >= threshold_of_displacement)
						{
							cout << "Trigger UP UP UP UP !!!!" << endl;
						}
						break;
					case 4:
						//Trigger Down Command
						if(y_displacement >= threshold_of_displacement)
						{
							cout << "Trigger DOWN DOWN DOWN DOWN !!!!" << endl;
						}
						break;

				}


				//reset current point
				//current = Point(-1,-1);
				current = resetPoint;
				//cout << "resetPoint -> x: " << resetPoint.x << " y: " << resetPoint.y << endl;				
				//cout << "current -> x: " << current.x << " y: " << current.y << endl;				


			}
		}

		//Draw ROI
		rectangle( drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		//Draw ROI for Command
		rectangle( drawing, roi_to_left, Scalar(255, 0, 212), 2, 8 );
		rectangle( drawing, roi_to_right, Scalar(255, 0, 212), 2, 8 );
		rectangle( drawing, roi_to_up, Scalar(0, 208, 255), 2, 8 );
		rectangle( drawing, roi_to_down, Scalar(0, 208, 255), 2, 8 );

		//Draw Origin Point and Current Point
		if( origin != resetPoint )
		{
			circle( drawing, origin, 5, Scalar(203,192,255), -1, 8, 0 );
		}
		if( current != resetPoint )
		{
			circle( drawing, current, 5, Scalar(203,192,255), -1, 8, 0 );
		}


		imshow("Histogram", histimg);
		imshow("CamShift tracking Object", drawing);


		waitKey(30);
	}

	return 0;
}

