#include "XtionCam.hpp"

int main()
{
	ICamera* cam = new XtionCam();
	cv::Mat depth;
	cv::Mat bgr;

	while (1) {
		cam->getDepthImage(depth);
		cam->getBgrImage(bgr);
		
		cam->get2Image(bgr, depth);
		imshow("depth", depth);
		imshow("bgr", bgr);
		
		if( cv::waitKey( 30 ) >= 0 )
			break;

	}

	delete cam;
}
