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


int main(int argc, char** argv){

	VideoCapture capture;
	Mat image, x_image, y_image, x_drawing, y_drawing, mask;
	//Mat dilate_element(10, 10, CV_8U, Scalar(1));

	//TrackBox
	Rect x_trackBox, y_trackBox;
	Point x_center_of_trackBox = Point(-1,-1);
	Point y_center_of_trackBox = Point(-1,-1);

	//track offset of object
	Point resetPoint = Point(-1,-1);
	Point origin = resetPoint;
	Point current = resetPoint;
	bool use_x = false;
	bool use_y = false;

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

	//condition
	int area_threshold = 500;

	for(;;)
	{
		//getDepthImage
		getDepthImage(capture, &image);

		imshow("Source", image);

		//initial x and y images
		x_image = Mat::zeros(image.size(), CV_8UC3);
		y_image = Mat::zeros(image.size(), CV_8UC3);
		//filterDepthImage
		filterDepthImage(&image, &x_image, (int)30, (int)40);
		filterDepthImage(&image, &y_image, (int)20, (int)40);

		//show depth image
		imshow("X -> Main Program for Depth Camera", x_image);
		imshow("Y -> Main Program for Depth Camera", y_image);

		//Create ROI Mask
		mask = Mat(image.size(), CV_8UC1, Scalar::all(255));
		mask(roi_rect).setTo(Scalar::all(0));
		imshow("Mask", mask);

		x_image.setTo( 0, mask);
		imshow("X with Mask -> Main Program for Depth Camera", x_image);
		y_image.setTo( 0, mask);
		imshow("Y with Mask -> Main Program for Depth Camera", y_image);


		//handle ROI
		//setDepthImageROI(&image, roi_rect_start_x, roi_rect_start_y, roi_rect_width, roi_rect_height);

		//Dilate
		//dilate(x_image, x_image, dilate_element, Point(-1, -1), 3);
		//dilate(y_image, y_image, dilate_element, Point(-1, -1), 3);
		//imshow("X Origin Depth", x_drawing);
		//imshow("Y Origin drawContours", y_drawing);

		//handle Contours
		vector< vector<Point> > x_contours;
		vector<Vec4i> x_hierarchy;
		vector< vector<Point> > y_contours;
		vector<Vec4i> y_hierarchy;
		//findContours
		getContours(&x_image, &x_contours, &x_hierarchy);
		getContours(&y_image, &y_contours, &y_hierarchy);

		//get largest contour
		int largest_contour_index_x = findLargestContourIndex(x_contours);
		int largest_contour_index_y = findLargestContourIndex(y_contours);
		//cout << "largest_contour_index: " << largest_contour_index << endl;

		//draw Contours
		x_drawing = Mat::zeros(x_image.size(), CV_8UC3);
		y_drawing = Mat::zeros(y_image.size(), CV_8UC3);
		//Using Contour tracking Object
		if(largest_contour_index_x != -1)
		{
			//X depth image exist
			drawContours(&x_drawing, &x_contours, &x_hierarchy, CV_FILLED, largest_contour_index_x, area_threshold);

			//get TrackBox for X
			x_trackBox = boundingRect(x_contours[largest_contour_index_x]);
			x_center_of_trackBox = Point( (x_trackBox.x + (x_trackBox.width/2)), (x_trackBox.y + (x_trackBox.height/2)) );

			//judge what is the first point in ROI
			if( origin == resetPoint && roi_rect.contains(x_center_of_trackBox) )
			{
				//find what is the command
				if(roi_to_left.contains(x_center_of_trackBox))
				{
					//Left
					gesture_command = 1;
					use_x = true;
					origin = x_center_of_trackBox;
					cout << "Entry Point is in X -> " << "origin.x: " << origin.x << " origin.y: " << origin.y << endl;

				}else if(roi_to_right.contains(x_center_of_trackBox))
				{
					//Right
					gesture_command = 2;
					use_x = true;
					origin = x_center_of_trackBox;
					cout << "Entry Point is in X -> " << "origin.x: " << origin.x << " origin.y: " << origin.y << endl;

				}			
			}

			if( use_x )
			{
				//set current Point
				current = x_center_of_trackBox;
			}

			//Draw focus retangle
			rectangle( x_drawing, x_trackBox, Scalar(0,255,0), 3, 8 );

			//Draw center of trackBox
			circle( x_drawing, x_center_of_trackBox, 5, Scalar(181,186,10), -1, 8, 0 );

		}

		if(largest_contour_index_y != -1)
		{
			//Y depth image exist	
			drawContours(&y_drawing, &y_contours, &y_hierarchy, CV_FILLED, largest_contour_index_y, area_threshold);

			//get TrackBox for Y
			y_trackBox = boundingRect(y_contours[largest_contour_index_y]);
			y_center_of_trackBox = Point( (y_trackBox.x + (y_trackBox.width/2)), (y_trackBox.y + (y_trackBox.height/2)) );

			//find what is thd commnad
			if( origin == resetPoint && roi_rect.contains(y_center_of_trackBox) )
			{

				//find what is the commnad
				if(roi_to_up.contains(y_center_of_trackBox))
				{
					//Up
					gesture_command = 3;
					use_y = true;
					origin = y_center_of_trackBox;
					cout << "Entry Point is in Y -> " << "origin.x: " << origin.x << " origin.y: " << origin.y << endl;

				}else if(roi_to_down.contains(y_center_of_trackBox))
				{
					//Down
					gesture_command = 4;
					use_y = true;
					origin = y_center_of_trackBox;
					cout << "Entry Point is in Y -> " << "origin.x: " << origin.x << " origin.y: " << origin.y << endl;

				}
			}

			if( use_y )
			{
				//set current point
				current = y_center_of_trackBox;
			}

			//Draw focus retangle
			rectangle( y_drawing, y_trackBox, Scalar(0,255,0), 3, 8 );

			//Draw center of trackBox
			circle( y_drawing, y_center_of_trackBox, 5, Scalar(181,186,10), -1, 8, 0 );


		}


		if( origin != resetPoint && current != resetPoint 
				&& ( (use_x && largest_contour_index_x == -1) || (use_y && largest_contour_index_y == -1) ) )
		{

			cout << "destination.x: " << current.x << " destination.y: " << current.y << endl;

			//count displacement
			x_displacement = abs(current.x - origin.x);
			y_displacement = abs(current.y - origin.y);
			cout << "x_displacement -> " << x_displacement << endl;
			cout << "y_displacement -> " << y_displacement << endl;

			//trigger xdotool command
			//judgment left right up and down

			//Method 1
			switch(gesture_command){
				case 1:
					//Trigger Left Command
					if(x_displacement >= threshold_of_displacement)
					{
						//cout << "Trigger LEFT LEFT LEFT LEFT !!!!" << endl;
						cout << "Trigger RIGHT RIGHT RIGHT RIGHT !!!!" << endl;
						//Do xdotool key command
						system("xdotool key Right");
					}
					break;
				case 2:
					//Trigger Right Command
					if(x_displacement >= threshold_of_displacement)
					{
						//cout << "Trigger RIGHT RIGHT RIGHT RIGHT !!!!" << endl;
						cout << "Trigger LEFT LEFT LEFT LEFT !!!!" << endl;
						//Do xdotool key command
						system("xdotool key Left");
					}
					break;
				case 3:	
					//Trigger Up Command
					if(y_displacement >= threshold_of_displacement)
					{
						cout << "Trigger UP UP UP UP !!!!" << endl;
						//Do xdotool key command
						system("xdotool key Up");
					}
					break;
				case 4:
					//Trigger Down Command
					if(y_displacement >= threshold_of_displacement)
					{
						cout << "Trigger DOWN DOWN DOWN DOWN !!!!" << endl;
						//Do xdotool key command
						system("xdotool key Down");
					}
					break;

			}


			//reset origin point
			origin = resetPoint;	
			//reset current point
			current = resetPoint;

			//reset gesture_command
			gesture_command = -1;
			//reset x and y flag
			use_x = false;
			use_y = false;

			//reset displacement
			x_displacement = -1;
			y_displacement = -1;

		}



		//Draw ROI
		rectangle( x_drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		rectangle( y_drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		//Draw ROI for Command
		rectangle( x_drawing, roi_to_left, Scalar(255, 0, 212), 2, 8 );
		rectangle( x_drawing, roi_to_right, Scalar(255, 0, 212), 2, 8 );
		rectangle( y_drawing, roi_to_up, Scalar(0, 208, 255), 2, 8 );
		rectangle( y_drawing, roi_to_down, Scalar(0, 208, 255), 2, 8 );

		//Draw Origin Point and Current Point
		if( origin != resetPoint )
		{
			if(use_x)
			{
				circle( x_drawing, origin, 5, Scalar(203,192,255), -1, 8, 0 );
			}else if(use_y)
			{
				circle( y_drawing, origin, 5, Scalar(203,192,255), -1, 8, 0 );
			}
		}

		if( current != resetPoint )
		{
			if(use_x)
			{
				circle( x_drawing, current, 5, Scalar(203,192,255), -1, 8, 0 );
			}else if(use_y)
			{
				circle( y_drawing, current, 5, Scalar(203,192,255), -1, 8, 0 );
			}
		}


		imshow("X -> Contour tracking Object", x_drawing);
		imshow("Y -> Contour tracking Object", y_drawing);


		waitKey(30);
	}

	return 0;
}

