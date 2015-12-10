#include "CPythonUtil.h"
#include <iostream>
#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define FRAME_WIDTH	1280
#define FRAME_HEIGHT	720

/*
 * Send a image to Python and then get the results back which are 4 integer
 */
void sendImage(CinvokePythonUtil& pyObj, const Mat& frame)
{

	
	pyObj.storeResult(pyObj.callMethod(pyObj.getInstance(),
			       	"sendImg",
			       	"(u, i, i, i)", frame.ptr(0), FRAME_HEIGHT, FRAME_WIDTH, 3));
	int x1, y1;
	int x2, y2;
	pyObj.parseTuple(pyObj.getResult(), "iiii", &x1, &y1, &x2, &y2);
	cout << "x1: " << x1 << " y1: " << y1 << endl;
	cout << "x2: " << x2 << " y2: " << y2 << endl;

}

/*
 * show the image that has been sent by sendImage 
 */
void showImage(CinvokePythonUtil& pyObj, const Mat& frame)
{

	
	pyObj.storeResult(pyObj.callMethod(pyObj.getInstance(),
			       	"showImg",
			       	"()"));
	
	char *gotStr = NULL;
	pyObj.parseArg(pyObj.getResult(), "s", &gotStr);
	cout << "Got result: " << gotStr << endl;
}


int main(int argc, char const *argv[])
{
	CinvokePythonUtil pyObj("hi", "Ludan");
        VideoCapture cap;
        Mat frame;

        frame.create(Size(FRAME_WIDTH, FRAME_HEIGHT), CV_8UC3);

        //if ( frame.isContinuous() ) cout << "yes" << endl;
        //Open RGB Camera
        cap.open(0);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

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

	sendImage(pyObj, frame);	
	showImage(pyObj, frame);	

	return 0;
}
