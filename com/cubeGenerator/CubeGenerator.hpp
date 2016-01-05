#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "../gestureControlModule/contours_operator.hpp"

#include <iostream>


using namespace cv;
using namespace std;

class CubeGenerator {
public:
    CubeGenerator();
    void generateCube(const Rect &hand_roi, const Mat &depth_frame, int cube[6]);
    ~CubeGenerator();
};
