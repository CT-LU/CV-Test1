#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "depth_image_generator.hpp"
#include "contours_operator.hpp"
#include <signal.h>
#include <syslog.h>

extern "C" {
#include "input_inf.h"
};

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;

#define START_DEPTH 65
#define END_DEPTH 80

//Gesture Command Type
enum GESTURE_COMMAND {
	NONE,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

/**
	Handle Ctrl+C Event in Linux
**/
void handleSig(int signo)
{
	if (signo == SIGINT) {
		cout << "received SIGINT, exiting..." << endl;
		syslog(LOG_INFO, "Gesture received SIGINT exiting...");
		//if (isBackground()) syslog(LOG_INFO, "received SIGINT, exiting...");
		//shutdown InputService Interface
		shutdownInterface();
		exit(0);
	}
	if (signo == SIGUSR1) {
		cout << "received SIGUSR1, exiting..." << endl;
		syslog(LOG_INFO, "Gesture received SIGUSR1 exiting...");
		//if (isBackground()) syslog(LOG_INFO, "received SIGUSR1, exiting...");
		//shutdown InputService Interface
		shutdownInterface();
		exit(0);
	}
}


int main(int argc, char** argv){

	//initial SIG variable
	struct sigaction sigIntHandler;
	
	//initial syslog
	openlog("gesture-aue", LOG_PID | LOG_CONS, LOG_USER);

	VideoCapture capture;
	Mat image, drawing, mask;

	//TrackBox
	Rect trackBox;
	Point center_of_trackBox = Point(-1,-1);

	//track offset of object
	Point resetPoint = Point(-1,-1);
	Point origin = resetPoint;
	Point current = resetPoint;

	//Calculate ROI parameter
	int source_image_width = 640;
	int source_image_height = 480;

	//ROI Rect
	int roi_rect_width = source_image_width * 0.5;//
	int roi_rect_height = source_image_height * 0.4;//
	int roi_rect_start_x = (source_image_width - roi_rect_width) * 0.5 ;
	int roi_rect_start_y = (source_image_height - roi_rect_height) * 0.5;
	
	//ROI for tracking
	Rect roi_rect(roi_rect_start_x, roi_rect_start_y, roi_rect_width, roi_rect_height);
	
	//record displacement
	int threshold_of_displacement = 0;

	//command tag
	GESTURE_COMMAND gesture_command = NONE;

	//getDepthCamera
	if(getDepthCamera(&capture) == -1)
	{
		cout << "Open Depth Camera Failed !!" << endl;
		syslog(LOG_ERR, "Open Depth Camera Failed !!");
		return -1;
	}else{
		cout << "Depth Camera Open Succeed !!" << endl;
		syslog(LOG_INFO, "Depth Camera Open Succeed !!");
	}


	//initial InputService
	DEV_TYPE input_type = GESTURE;
	if( initInterface(input_type) != SUCCESS ){
		cout << "Error!! initInterface failure!" << endl;
		syslog(LOG_ERR, "Gesture -> do inputservice initInterface failure!");
		return -1;
	}

	//handle Ctrl+C event
	sigIntHandler.sa_handler = handleSig;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	if (sigaction(SIGINT, &sigIntHandler, NULL) != 0) {
		cout << "can't catch SIGINT" << endl;
		syslog(LOG_ERR, "Gesture can't catch SIGINT");
	}
	if (sigaction(SIGUSR1, &sigIntHandler, NULL) != 0) {
		cout << "can't catch SIGUSR1" << endl;
		syslog(LOG_ERR, "Gesture can't catch SIGUSR1");
	}


	//condition
	int area_threshold = 0;

	for(;;)
	{
		//getDepthImage
		getDepthImage(capture, &image);
		//imshow("Source", image);

		//filterDepthImage
		filterDepthImage(&image, &image, START_DEPTH, END_DEPTH);  //(int)65, (int)80

		//show depth image
		//imshow("Main Program for Depth Camera", image);

		//Create ROI Mask
		mask = Mat(image.size(), CV_8UC1, Scalar::all(255));
		mask(roi_rect).setTo(Scalar::all(0));
		//imshow("Mask", mask);

		image.setTo( 0, mask);
		//imshow("X with Mask -> Main Program for Depth Camera", image);

		//handle Contours
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;

		//findContours
		getContours(&image, &contours, &hierarchy);

		//get largest contour
		int largest_contour_index = findLargestContourIndex(contours);
		//cout << "largest_contour_index: " << largest_contour_index << endl;

		//clear drawing
		if(origin == resetPoint){
			drawing = Mat::zeros(image.size(), CV_8UC3);
		}

		//Using Contour tracking Object
		if(largest_contour_index != -1)
		{
			//depth image exist
			//drawContours(&drawing, &contours, &hierarchy, CV_FILLED, largest_contour_index, area_threshold);

			//get TrackBox, TrackBox is boundingRectangle of the largest contour
			trackBox = boundingRect(contours[largest_contour_index]);
			//center point of trackBox
			center_of_trackBox = Point( (trackBox.x + (trackBox.width/2)), (trackBox.y + (trackBox.height/2)) );

			/**
				if origin == resetPoint and center_of_trackBox is inside roi rectangle
					set center_of_trackBox as origin point and current point
				else if origin is setted and center_of_trackBox is inside roi rectangle
					set center_of trackBox as current point
			**/
			if(origin == resetPoint && roi_rect.contains(center_of_trackBox))
			{
				origin = center_of_trackBox;
				current = origin;
				//cout << "Entry Point -> origin( " << origin.x << ", " << origin.y << ")" << endl;
			}else if(roi_rect.contains(center_of_trackBox)){
				current = center_of_trackBox;
			}

			//Draw center of trackBox
			circle( drawing, center_of_trackBox, 5, Scalar(181,186,10), -1, 8, 0 );

		}

		/**
			if 
				1. origin point and current point are setted
				2. origin point and current point "are not" the same point
				3. can not find any contour in current frame
			then
				1. calculate value of tantheta
				2. judge gesture command
				3. after judgement, reset gesture judgement environment variable
		**/
		if( origin != resetPoint && current != resetPoint 
				&& largest_contour_index == -1 
				&& origin != current )
		{
			//cout << "Destination Point -> destination( " << current.x << ", " << current.y << ")" << endl;

			//value of tantheta
			float tantheta = (float)(abs(current.y - origin.y))/(float)(abs(current.x - origin.x));

			//judgement for gesture
			if(tantheta <= sqrt(3))
			{
				//Left and Right
				if((current.x - origin.x) >= threshold_of_displacement)
				{
					//Left
					gesture_command = LEFT;
				}else{
					//Right
					gesture_command = RIGHT;
				}
			}else{
				//Up and Down
				if((current.y - origin.y) >= threshold_of_displacement)
				{
					//Down
					gesture_command = DOWN;
				}else{
					//Up
					gesture_command = UP;
				}
			}

			//Trigger Command
			switch(gesture_command){
				case RIGHT:
					//Trigger Right Command
					cout << "Trigger RIGHT RIGHT RIGHT RIGHT !!!!" << endl;
					syslog(LOG_INFO, "Trigger RIGHT RIGHT RIGHT RIGHT !!!!");
					//Do InputService key command
					sendMsg(KEY_MSG, "Right");
					syslog(LOG_INFO, "Gesture sendMsg(Right) to inputservice !");
					break;
				case LEFT:
					//Trigger Left Command
					cout << "Trigger LEFT LEFT LEFT LEFT !!!!" << endl;
					syslog(LOG_INFO, "Trigger LEFT LEFT LEFT LEFT !!!!");
					//Do InputService key command
					sendMsg(KEY_MSG, "Left");
					syslog(LOG_INFO, "Gesture snedMsg(Left) to inputservice !");
					break;
				case UP:	
					//Trigger Up Command
					cout << "Trigger UP UP UP UP !!!!" << endl;
					syslog(LOG_INFO, "Trigger UP UP UP UP !!!!");
					//Do InputService key command
					sendMsg(KEY_MSG, "Up");
					syslog(LOG_INFO, "Gesture snedMsg(Up) to inputservice !");
					break;
				case DOWN:
					//Trigger Down Command
					cout << "Trigger DOWN DOWN DOWN DOWN !!!!" << endl;
					syslog(LOG_INFO, "Trigger DOWN DOWN DOWN DOWN !!!!");
					//Do InputService key command
					sendMsg(KEY_MSG, "Down");
					syslog(LOG_INFO, "Gesture snedMsg(Down) to inputservice !");
					break;

			}

			//reset environment variable
			origin = resetPoint;
			current = resetPoint;
			gesture_command = NONE;
		}
		/**
			this is a special case: there is only one point, gesture judgement can not be done !!
			if 
				1. origin point and current point are setted
				2. origin point and current point "are" the same point
				3. can not find any contour in current frame
			then
				1. drop this gesture judgement, reset gesture environment variable
		**/
		else if(origin != resetPoint && current != resetPoint && origin == current && largest_contour_index == -1 ){
			//reset environment variable
			origin = resetPoint;
			current = resetPoint;
			gesture_command = NONE;
		}


		//Draw ROI
		rectangle( drawing, roi_rect, Scalar(181,186,10), 2, 8 );

		//Show the result image
		imshow("Contour tracking Object", drawing);


		waitKey(30);
	}

	//shutdown InputService Interface
	shutdownInterface();
	
	return 0;
}

