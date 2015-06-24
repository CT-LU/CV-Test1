
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string>

using namespace std;
using namespace cv;

static void help()
{
    printf("\n"
            "This program demonstrated a simple method of connected components clean up of background subtraction\n"
            "When the program starts, it begins learning the background.\n"
            "You can toggle background learning on and off by hitting the space bar.\n"
            "Call\n"
            "./segment_objects [video file, else it reads camera 0]\n\n");
}


int main(int argc, char** argv)
{
    VideoCapture cap;
    bool update_bg_model = true;

    help();

    if( argc < 2 )
        cap.open(0);
    else
        cap.open(std::string(argv[1]));

    if( !cap.isOpened() )
    {
        printf("\nCan not open camera or video file\n");
        return -1;
    }

    Mat tmp_frame, bgmask;

    cap >> tmp_frame;
    if(tmp_frame.empty())
    {
        printf("can not read data from the video source\n");
        return -1;
    }

    namedWindow("video", 1);
    namedWindow("segmented", 1);

    Ptr<BackgroundSubtractorMOG2> bgsubtractor=createBackgroundSubtractorMOG2();
    bgsubtractor->setVarThreshold(30);
    bgsubtractor->setNMixtures(1);
    bgsubtractor->setDetectShadows(false);
    bgsubtractor->setBackgroundRatio(1.0);
    //cout << bgsubtractor->getBackgroundRatio() << endl;

    for(;;)
    {
        cap >> tmp_frame;
        if( tmp_frame.empty() )
            break;
        
	bgsubtractor->apply(tmp_frame, bgmask, update_bg_model ? -1 : 0);
#if 1
    Mat vertical = bgmask.clone();
    // Specify size on vertical axis
    int verticalsize = vertical.rows / 30;

    // Create structure element for extracting vertical lines through morphology operations
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size( 1,verticalsize));

    // Apply morphology operations
    erode(bgmask, bgmask, verticalStructure, Point(-1, -1));
    dilate(bgmask, bgmask, verticalStructure, Point(-1, -1));
#endif

        //imshow("video", tmp_frame);
        imshow("segmented", bgmask);
        int keycode = waitKey(20);
        if( keycode == 27 )
            break;
        
	if( keycode == ' ' )
        {
            update_bg_model = !update_bg_model;
            printf("Learn background is in state = %d\n",update_bg_model);
        }
    }

    return 0;
}
