#include <iostream>
#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "hand_detect.hpp"

#include "CPythonUtil.h"

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

using namespace std;
using namespace cv;


int main(int argc, char const *argv[])
{
    //Initial Hand Detector
    HandDetector hand_detector = HandDetector();
    
    //Hand Detector TEST CODE
    //hand_detector.detect();

    
    VideoCapture cap;
    Mat frame;
    
    //Create Frame
    frame.create(Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC3);

    //if ( frame.isContinuous() ) cout << "yes" << endl;
    //Open RGB Camera
    cap.open(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
    
    //Chrck Camera is OK
    if( !cap.isOpened() )
    {
        cout << "Can not open camera !!" << endl;
        exit(1);
    }

    //read frame
    cap >> frame;
    if( frame.empty() )
    {
        cout << "Can not read data from the Camera !!" << endl;
        exit(1);
    }

    for(;;){
        cap >> frame;
        if(frame.empty()){
            break;
            //exit(1);
        }
        
        //Run Hand Detect
        int rect[4] = {0};
        hand_detector.detect(frame, rect);
        Rect hand5(rect[0], rect[1], (rect[2]-rect[0]), (rect[3]-rect[1]));
        //Draw Rectangle
        rectangle(frame, hand5, Scalar(0, 255, 0), 3, 8, 0);
        //Show Image
        imshow("Detect Result", frame);

        //Key Control
        int key_code = waitKey(30);
        if(key_code == 27) break;
    }
    
    if(cap.isOpened())
        cap.release();

	return 0;
}
