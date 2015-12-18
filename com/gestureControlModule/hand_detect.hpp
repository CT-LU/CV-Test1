#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

#include "CPythonUtil.h"

using namespace cv;
using namespace std;

class HandDetector {
public:
    HandDetector();
    void detect(const Mat& frame, int rect[4]);
    //void detect();
    ~HandDetector();

private:
    //Initial Python Interface
    CinvokePythonUtil *pyObj;
};
