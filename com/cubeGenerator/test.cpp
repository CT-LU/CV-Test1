#include <iostream>
#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "../invokeDetectModule/hand_detect.hpp"
#include "../invokeDetectModule/CPythonUtil.hpp"
#include <../camera/XtionCam.hpp>

#include "CubeGenerator.hpp"

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

using namespace std;
using namespace cv;


int main(int argc, char const *argv[])
{
    //Initial Hand Detector
    HandDetector hand_detector = HandDetector("../fast-rcnn/tools/");
    
    //Initial Cube Generator
    CubeGenerator cube_generator = CubeGenerator();

    //Create Frame
    Mat bgr_frame, depth_frame, mask;
    bgr_frame.create(Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC3);

    ICamera* cam = new XtionCam();
    int rect[4] = {0};
    int cube[6] = {0};
    
    for(;;){
        //get Depth and BGR frame
        cam->get2Image(bgr_frame, depth_frame);
        
        if(bgr_frame.empty()){
            break;
        }
        
        //Run Hand Detect
        rect[4] = {0};
        cube[6] = {0};
        hand_detector.detect(bgr_frame, rect);

        if(rect[0] != 0 && rect[1] != 0 && rect[2] != 0 && rect[3] != 0){
            Rect hand5(rect[0], rect[1], (rect[2]-rect[0]), (rect[3]-rect[1]));
            //Draw Rectangle
            rectangle(bgr_frame, hand5, Scalar(0, 255, 0), 3, 8, 0);
            //Generate Cube
            cube_generator.generateCube(hand5, depth_frame, cube);
	        threshold(depth_frame, depth_frame, cube[4], 255, THRESH_TOZERO);
	        threshold(depth_frame, depth_frame, cube[5], 255, THRESH_TOZERO_INV);
            //Create ROI Mask
            Rect roi_rect(cube[0], cube[1], (cube[2]-cube[0]), (cube[3]-cube[1]));
            mask = Mat(depth_frame.size(), CV_8UC1, Scalar::all(255));
            mask(roi_rect).setTo(Scalar::all(0));
            depth_frame.setTo( 0, mask);
        }

        //Show Image
        imshow("BGR Detect Result", bgr_frame);
        imshow("Depth Image Result", depth_frame);

        //Key Control
        int key_code = waitKey(30);
        if(key_code == 27) break;
    }

    //Release
    delete cam;

	return 0;
}
