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

HandDetector::HandDetector(const char* fast_rcnn_path){
	//Initial Python Interface
	pyObj = new CinvokePythonUtil("hi", "Ludan", fast_rcnn_path);
}

void HandDetector::detect(const Mat& frame, int rect[4]){
	
    pyObj->storeResult(pyObj->callMethod(pyObj->getInstance(),
				"call_fast_rcnn_frame",
				"(O, i, i, i)", 
				pyObj->createPyArray(frame.ptr(0), frame.rows, frame.cols, frame.channels()), 
				frame.rows, frame.cols, frame.channels()));


	pyObj->parseTuple(pyObj->getResult(), 
			"iiii", &rect[0], &rect[1], &rect[2], &rect[3]);
	//debug message
	cout << "xmin: " << rect[0] << " ymin: " << rect[1] << endl;
	cout << "xmax: " << rect[2] << " ymax: " << rect[3] << endl;

}

HandDetector::~HandDetector(){
	delete pyObj;
}

