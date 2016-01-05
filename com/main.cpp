#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "./camera/XtionCam.hpp"
#include "./gestureControlModule/gesture_control.hpp"
#include "./gestureControlModule/contours_operator.hpp"
#include "./MaskWithCudaGmm/MaskWithCudaGmm.hpp"
#include "./invokeDetectModule/CPythonUtil.hpp"
#include "./invokeDetectModule/hand_detect.hpp"
#include "./cubeGenerator/CubeGenerator.hpp"

#include <signal.h>
#include <syslog.h>
#include <time.h>

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

    //Initial Hand Detector
    HandDetector hand_detector = HandDetector("./fast-rcnn/tools/");
    
    //Initial Cube Generator
    CubeGenerator cube_generator = CubeGenerator();

    //Get Camera
    ICamera* cam = new XtionCam();
    
    //Mat bgr_image;
    Mat bgr_frame, depth_frame;
    //Create Frame
    bgr_frame.create(Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC3);
    //Hand Rect
    int rect[4] = {0};
    int cube[6] = {0};
    bool get_roi = false;
    
    //GMM Initial
    cam->getBgrImage(bgr_frame);
    //GMM Filter
	MaskWithCudaGmm masker(bgr_frame.ptr(0));

    //Profiler
    double START, END;
    double DETECT_START, DETECT_END;

    for(;;){
        START = clock();
        if(!get_roi){
            //get Depth and BGR frame
            cam->get2Image(bgr_frame, depth_frame);

            DETECT_START = clock();
            //GMM Filter
		    Mat mask_frame = Mat::zeros(bgr_frame.size(), CV_8UC3);
		    //Get GMM Mask
            masker.maskWithCudaGmm(bgr_frame, mask_frame);
            imshow("GMM-Final", mask_frame);

            //Run Hand Detect
            rect[4] = {0};
            cube[6] = {0};
            hand_detector.detect(mask_frame, rect);
            DETECT_END = clock();

            //Show detect time
            cout << "Detect time per frame: " << (DETECT_END - DETECT_START) / CLOCKS_PER_SEC << endl;

            if(rect[0] != 0 && rect[1] != 0 && rect[2] != 0 && rect[3] != 0){
              
                Rect hand5(rect[0], rect[1], (rect[2]-rect[0]), (rect[3]-rect[1]));
                
                //get Cube ROI
                //Generate Cube
                cube_generator.generateCube(hand5, depth_frame, cube);
                //set Cube ROI
                gesture.setGestureROI(cube[0], cube[1], cube[2], cube[3], 
                                      cube[4], cube[5]);
                //Set ROI Flag
                get_roi = true;

                //Draw Rectangle
                rectangle(bgr_frame, hand5, Scalar(0, 255, 0), 3, 8, 0);
                rectangle(bgr_frame, gesture.roi_rect, Scalar(0, 0, 255), 3, 8, 0);
                rectangle(depth_frame, hand5, Scalar(255, 255, 255), 3, 8, 0);
                rectangle(depth_frame, gesture.roi_rect, Scalar(255, 255, 255), 3, 8, 0);
            }
            //Show Image
		    imshow("BGR", bgr_frame);
            imshow("DEPTH", depth_frame);
        }else{
            //Process Next Depth Frame
            cam->getDepthImage(depth_frame);
            gesture.processNextFrame(depth_frame);

            //Draw ROI
            rectangle( gesture.drawing, gesture.roi_rect, Scalar(181,186,10), 2, 8 );
            rectangle( gesture.debug_frame, gesture.roi_rect, Scalar(181,186,10), 2, 8 );
            //Show Depth result image
            imshow("Contour tracking Object", gesture.drawing);
            imshow("Debug Depth", gesture.debug_frame);
        }

        END = clock();
        //Show frame time
        cout << "Time per frame: " << (END - START) / CLOCKS_PER_SEC << endl;
        
        //Key Control
		int key_code = waitKey(30);
        if(key_code == 27) break;
        if(key_code == 32){
            //Change ROI position
            cout << "Reset 'get_roi' flag" << endl;
            get_roi = false;
			gesture.drawing = Mat::zeros(gesture.debug_frame.size(), CV_8UC3);
        }
    }

    //ShutdownGesture
    gesture.shutdownGesture();

    //Release
    delete cam;
    
    return 0;
}

