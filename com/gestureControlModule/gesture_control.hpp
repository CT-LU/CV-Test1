//Gesture Controller Header
//#define ENABLE_INPUT_SERVICE

#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "contours_operator.hpp"

#include <signal.h>
#include <syslog.h>

#ifdef ENABLE_INPUT_SERVICE
extern "C" {
#include "input_inf.h"
};
#endif

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;

//Gesture Command Type
enum GESTURE_COMMAND {
	NONE,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class GestureControl {
public:
    GestureControl();
    void setGestureROI(int xmin, int ymin, int xmax, int ymax, int depth_min, int depth_max);
#if 0
    void generateStaticGestureROI();
    void generateGestureROI(const Rect &hand_roi, const Mat &depth_frame);
#endif
    void trackingObjectByContour(int largest_contour_index);
    void resetGestureEnviroment();
    void getGestureDirection();
    void sendGestureCommand();
    void filterDepthImage(Mat &src_image, Mat &dst_image,
                          int start_depth, int end_depth);
    void processNextFrame(Mat &depth_frame);
    void shutdownGesture();
    
    //OpenCV object
	Mat drawing, mask, debug_frame;
    Rect roi_rect;

    //track offset of object
	const Point resetPoint = Point(-1,-1);
	Point origin = resetPoint;
	Point current = resetPoint;
	//Point center_of_roi = resetPoint;

    //hand depth
    //int depth_base = 0;
    int start_depth = 0;
    int end_depth = 0;

	//record displacement
	int threshold_of_displacement = 0;
	int area_threshold = 0;

private:
    
    //TrackBox
	Rect trackBox;
	Point center_of_trackBox = Point(-1,-1);
    
    //handle Contours
    int largest_contour_index = 0;
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    
    //command tag
	GESTURE_COMMAND gesture_command = NONE;
    
#ifdef ENABLE_INPUT_SERVICE
    //InputService
	DEV_TYPE input_type = GESTURE;
#endif

};

