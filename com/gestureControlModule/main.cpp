//Gesture Main
//#define RGB_CAMERA
#define DEPTH_CAMERA

#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "gesture_control.hpp"
#include "hand_detect.hpp"

#include "MaskWithCudaGmm.hpp"

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

#ifdef DEPTH_CAMERA
    //Control Gesture Flow
    if(gesture.openDepthCamera() == -1){
        //OpenCamera failed
        //return -1;
        exit(1);
    }
#else
    VideoCapture cap(0);

    if(cap.isOpened()){
        cout << "RGB Camera open is OK" << endl;
    }else{
        cout << "RGB Camera open is Failed" << endl;
        exit(1);
    }
#endif

    //Initial Hand Detector
    HandDetector hand_detector = HandDetector();
    
    //Mat bgr_image;
    Mat frame, depth_frame;
    //Create Frame
    frame.create(Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC3);
    //Hand Rect
    int rect[4] = {0};
    bool get_roi = false;
    
    //GMM Initial
#ifdef DEPTH_CAMERA
    //get Depth and BGR frame
    gesture.getNextDetectFrame(depth_frame, frame);
#else
    cap >> frame;
    if(frame.empty()){
        cout << "get RGB Frame Error";
        break;
    }
#endif

    //GMM Filter
	MaskWithCudaGmm masker(frame.ptr(0));

    for(;;){
        if(!get_roi){
#ifdef DEPTH_CAMERA
            //get Depth and BGR frame
            gesture.getNextDetectFrame(depth_frame, frame);
#else
            cap >> frame;
            if(frame.empty()){
                cout << "get RGB Frame Error";
                break;
            }
#endif
 
            //GMM Filter
		    Mat mask_frame = Mat::zeros(frame.size(), CV_8UC3);
		    //Get GMM Mask
            masker.maskWithCudaGmm(frame, mask_frame);

            imshow("GMM", mask_frame);

            //Run Hand Detect
            rect[4] = {0};
            hand_detector.detect(frame, rect);

            if(rect[0] != 0 && rect[1] != 0 && rect[2] != 0 && rect[3] != 0){
                Rect hand5(rect[0], rect[1], (rect[2]-rect[0]), (rect[3]-rect[1]));
                
                //Create Dynamic ROI
                gesture.generateGestureROI(hand5, depth_frame);

                //Set ROI Flag
                get_roi = true;

                //Draw Rectangle
                rectangle(frame, hand5, Scalar(0, 255, 0), 3, 8, 0);
                rectangle(frame, gesture.roi_rect, Scalar(0, 0, 255), 3, 8, 0);
                rectangle(depth_frame, hand5, Scalar(255, 255, 255), 3, 8, 0);
                rectangle(depth_frame, gesture.roi_rect, Scalar(255, 255, 255), 3, 8, 0);
            }
            //Show Image
		    imshow("BGR", frame);
            imshow("DEPTH", depth_frame);
        }else{
            //Process Next Depth Frame
            gesture.processNextFrame();
            //Draw ROI
            rectangle( gesture.drawing, gesture.roi_rect, Scalar(181,186,10), 2, 8 );
            rectangle( gesture.debug_image, gesture.roi_rect, Scalar(181,186,10), 2, 8 );
            //Show Depth information
            char str_msg[20];
            sprintf(str_msg, "Depth: %d", gesture.depth_base);
            putText(gesture.drawing, str_msg, Point(gesture.roi_rect.x, gesture.roi_rect.y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0));
            putText(gesture.debug_image, str_msg, Point(gesture.roi_rect.x, gesture.roi_rect.y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255));
            //cout << "Depth_Base is: " << gesture.depth_base << "  Start_Depth: " << gesture.start_depth << "  End_Depth: " << gesture.end_depth << endl;

            //Show Depth result image
            //imshow("Source Depth Image", gesture.image);
            imshow("Contour tracking Object", gesture.drawing);
            imshow("Debug Depth", gesture.debug_image);
        }

        
        //Key Control
		int key_code = waitKey(30);

        /*
        if(get_roi){
            key_code = waitKey(0);
        }else{
            key_code = waitKey(30);
        }
        cout << "key_code: " << key_code << endl;
        */

        if(key_code == 27) break;
        if(key_code == 32){
            //Change ROI position
            cout << "Reset 'get_roi' flag" << endl;
            get_roi = false;
			gesture.drawing = Mat::zeros(gesture.image.size(), CV_8UC3);
        }
    }

    //ShutdownGesture
    gesture.shutdownGesture();
    
    return 0;
}

