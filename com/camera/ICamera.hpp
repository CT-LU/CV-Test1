#ifndef __ICAMERA_H__
#define __ICAMERA_H__

#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

/*
 * Interface of cameras, all kinds of camera need to implement this
 */
class ICamera {
	public:
		virtual int getBgrImage(cv::Mat &bgr_image) = 0;
		virtual int getDepthImage(cv::Mat &depth_image) = 0;
		virtual int get2Image(cv::Mat &bgr_image, cv::Mat &depth_image) = 0;
};

#endif
