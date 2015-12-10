//Gesture Main

#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "gesture_control.hpp"

#include <signal.h>
#include <syslog.h>

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;


int main(int argc, char** argv){

	//initial syslog
	openlog("gesture-main", LOG_PID | LOG_CONS, LOG_USER);
    //Create gesture object   
    GestureControl gesture = GestureControl();
    //Control Gesture Flow
    if(gesture.openDepthCamera() == -1){
        //OpenCamera failed
        return -1;
    }
    gesture.roi_rect = gesture.getGestureROI();
    
    for(;;){
        //Next Frame
        gesture.processNextFrame();
        //Draw ROI
		rectangle( gesture.drawing, gesture.roi_rect, Scalar(181,186,10), 2, 8 );
		//Show the result image
		imshow("Contour tracking Object", gesture.drawing);
        //Key Control 
		int key_code = waitKey(30);
        if(key_code == 27) break;
        if(key_code == 32){
            //Change ROI position
            gesture.roi_rect = gesture.getGestureROI();
			gesture.drawing = Mat::zeros(gesture.image.size(), CV_8UC3);
        }
    }
    //ShutdownGesture
    gesture.shutdownGesture();
    
    return 0;
}

