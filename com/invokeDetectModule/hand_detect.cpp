#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

#include "CPythonUtil.hpp"
#include "hand_detect.hpp"

using namespace cv;
using namespace std;

HandDetector::HandDetector(){
	//Initial Python Interface
	//pyObj = new CinvokePythonUtil("fast_rcnn_c_interface", "Fast_RCNN_C_Interface");
	pyObj = new CinvokePythonUtil("hi", "Ludan");
}

void HandDetector::detect(const Mat& frame, int rect[4]){
	//void HandDetector::detect(){
	/*
	   pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
	   "create_model",
	   "()"));

	   pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
	   "detect_object_from_c",
	   "(u, i, i, i)", 
	   frame.ptr(0), frame.rows, frame.cols, frame.channels()));

*/

	/*
	   pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
	   "sendImg",
	   "(u, i, i, i)", 
	   frame.ptr(0), frame.rows, frame.cols, frame.channels()));
	   */

	/*
	   pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
	   "call_fast_rcnn_loop",
	   "()"));
	   */
#if 0
	pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
				"call_fast_rcnn_frame",
				"(u, i, i, i)", 
				frame.ptr(0), frame.rows, frame.cols, frame.channels()));
#endif
	pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
				"sendImg",
				"(O, i, i, i)", 
				pyObj->createPyArray(frame.ptr(0), frame.rows, frame.cols, frame.channels()), 
				frame.rows, frame.cols, frame.channels()));


	//int rect[4] = {0};
	pyObj->parseTuple(pyObj->getResult(), 
			"iiii", &rect[0], &rect[1], &rect[2], &rect[3]);
	//debug message
	cout << "xmin: " << rect[0] << " ymin: " << rect[1] << endl;
	cout << "xmax: " << rect[2] << " ymax: " << rect[3] << endl;


	/*
	//ShowImg
	pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
	"showImg",
	"()"));

	char *gotStr = NULL;
	pyObj->parseArg(pyObj->getResult(), "s", &gotStr);
	cout << "Got Result: " << gotStr << endl;
	*/
}

HandDetector::~HandDetector(){
	delete pyObj;
}

