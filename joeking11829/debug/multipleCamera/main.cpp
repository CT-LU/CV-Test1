#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <signal.h>
#include <syslog.h>

#include "depth_image_generator.hpp"

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;


int main(int argc, char** argv){

	//VideoCapture cap_one, cap_two;
	Mat frame_one, frame_two, frame_depth;

	/*
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
	 */

	//Open RGB Camera One
	//cap_one.open(0);
	VideoCapture cap_one(0);
    
    //get VideoCapture Config
    //cap_one.set(3, 1280);
    //cap_one.set(4, 720);
    cout << "Camera  Width: " << cap_one.get(3) << "  Height: " << cap_one.get(4) << endl;

	if( !cap_one.isOpened() )
	{
		cout << "Can not open Camera One !!" << endl;
		return -1;
	}

	//read frame
	cap_one >> frame_one;
	if( frame_one.empty() )
	{
		cout << "Can not read data from the Camera One !!" << endl;
		return -1;
	}
	
	/*
	//Open RGB Camera Two
	//cap_two.open(1);
	VideoCapture cap_two(1);
	
	if( !cap_two.isOpened() )
	{
		cout << "Can not open Camera Two !!" << endl;
		return -1;
	}

	//read frame
	cap_two >> frame_two;
	if( frame_two.empty() )
	{
		cout << "Can not read data from the Camera Two !!" << endl;
		return -1;
	}
	*/

	//Open Depth Camera
    /*
	VideoCapture cap_depth;
	if(getDepthCamera(&cap_depth) == -1)
	{
		cout << "Open Depth Camera Failed !!" << endl;
		return -1;
	}
    */


	for(;;)
	{
		//Get RGB Image from Camera One
		cap_one >> frame_one;
		if( frame_one.empty() )
		{
			break;
		}
		

		/*
		//Get RGB Image from Camera Two
		cap_two >> frame_two;
		if( frame_two.empty() )
		{
			break;
		}
		*/


		//debug Camera One
		imshow("Source One", frame_one);

		//debug Camera Two
		//imshow("Source Two", frame_two);

		//Get Depth Image from Depth Camera
		//getDepthImage(cap_depth, &frame_depth);
		//imshow("Source Depth", frame_depth);

		//User Key Input
		waitKey(30);
	}

	return 0;
}

