#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "gesture_control.hpp"
#include "XtionCam.hpp"

#include <signal.h>
#include <syslog.h>

#include <iostream>
#include <ctype.h>
#include <math.h>

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

using namespace cv;
using namespace std;


int main(int argc, char** argv){

    //Create gesture object   
    GestureControl gesture = GestureControl();

    //create ROI
    int roi_rect_width = FRAME_WIDTH * 0.5;
    int roi_rect_height = FRAME_HEIGHT * 0.4;
    int xmin = (FRAME_WIDTH - roi_rect_width) * 0.5 ;
    int ymin = (FRAME_HEIGHT - roi_rect_height) * 0.5;
    int xmax = xmin + roi_rect_width;
    int ymax = ymin + roi_rect_height;
    //Set Depth range
    int start_depth = 65;
    int end_depth = 80;

    gesture.setGestureROI(xmin, ymin, xmax, ymax, 
                          start_depth, end_depth);
    
    //Get Camera
    Mat depth_frame;
	ICamera* cam = new XtionCam();

    for(;;){
        //Get Depth Frame
		cam->getDepthImage(depth_frame);
        //Process Next Depth Frame
        gesture.processNextFrame(depth_frame);
        //Draw ROI
        rectangle( gesture.drawing, gesture.roi_rect, Scalar(181,186,10), 2, 8 );
        //Show Depth result image
        imshow("Contour tracking Object", gesture.drawing);
        
        //Key Control
		int key_code = waitKey(30);
        if(key_code == 27) break;
    }

    //ShutdownGesture
    gesture.shutdownGesture();
    
    return 0;
}

