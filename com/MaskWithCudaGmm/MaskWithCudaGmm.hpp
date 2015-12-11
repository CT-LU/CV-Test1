#ifndef _MASKWITHCUDAGMM_H_
#define _MASKWITHCUDAGMM_H_

#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <vector>
#include "gmmWithCuda.h"

using namespace cv;
using namespace std;

class MaskWithCudaGmm {
	private:
		int getContours(const Mat &image, vector< vector<Point> > &contours, vector<Vec4i> &hierarchy);
		/*
		 * gmm_frame is the result from CUDA-GMM, input is original image, output will be a masked frame 
		 */
		void performMask(const Mat& gmm_frame, const Mat& input, Mat& output);
	public:
		/*
		 * constuctor is to initialize cuda, need the first frame to establish gmm models
		 */
		MaskWithCudaGmm(const unsigned char* frame);
		~MaskWithCudaGmm();
		/*
		 * in_frame is original image from for example web cam, out_frame is expected as masked frame after this function
		 */
		void maskWithCudaGmm(const Mat& in_frame, Mat& out_frame);
};

#endif

