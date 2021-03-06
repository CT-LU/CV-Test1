#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

#include "CPythonUtil.hpp"

using namespace cv;
using namespace std;

class HandDetector {
public:
    HandDetector(const char* fast_rcnn_path);
    void detect(const Mat& frame, int rect[4]);
    ~HandDetector();

private:
    //Initial Python Interface
    CinvokePythonUtil *pyObj;
};
