#ifndef __XTION_CAM_H__
#define __XTION_CAM_H__

#include "ICamera.hpp"

class XtionCam : public ICamera {
	public:
		XtionCam();
		~XtionCam();

		virtual int getBgrImage(cv::Mat &bgr_image);
		virtual int getDepthImage(cv::Mat &depth_image);
		virtual int get2Image(cv::Mat &bgr_image, cv::Mat &depth_image);
	protected:
		cv::VideoCapture capture;
};

#endif
