#include "gmmWithCuda.h"
#include "MaskWithCudaGmm.hpp"

/*
 * constuctor is to initialize cuda, need the first frame to establish gmm models
 */
MaskWithCudaGmm::MaskWithCudaGmm(const unsigned char* frame) {
	gpu_initialize_gmm(frame);
}

MaskWithCudaGmm::~MaskWithCudaGmm() {
	gpu_free_gmm();
}

int MaskWithCudaGmm::getContours(const Mat &image, vector< vector<Point> > &contours, vector<Vec4i> &hierarchy){

    //Erode
    Mat elementB(3, 3, CV_8U, Scalar(1));
    erode(image, image, elementB, Point(-1, -1), 1);

	//MorphologyEx
	Mat elementA(3, 3, CV_8U, Scalar(1));
	morphologyEx(image, image, cv::MORPH_CLOSE, elementA);

    //imshow("MorEX", image);

	//findContours
	//get Contours external
	findContours(image, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	return 0;
}

/*
 * gmm_frame is the result from CUDA-GMM, input is original image, output will be a masked frame 
 */
void MaskWithCudaGmm::performMask(const Mat& gmm_frame, const Mat& input, Mat& output)
{
	//handle Contours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//findContours
	getContours(gmm_frame, contours, hierarchy);
	
	Mat mask = Mat::zeros(input.size(), CV_8UC1);
	for (size_t i = 0; i < contours.size(); i++)
	{

		if ((contourArea((contours)[i]) <= 100)){
			continue;
		}

		//drawContours( drawing, contours, int(i), Scalar(255, 255, 255), CV_FILLED, 8, hierarchy, 0, Point() );
		rectangle(mask, boundingRect(contours[i]), Scalar(255,255,255), CV_FILLED, 8, 0);
	}
	//imshow("Contours", mask);
	input.copyTo(output, mask);
}

/*
 * in_frame is original image from for example web cam, out_frame is expected as masked frame after this function
 */
void MaskWithCudaGmm::maskWithCudaGmm(const Mat& in_frame, Mat& out_frame) {
		
	//GMM output
	Mat gmm_frame;
	gmm_frame.create(in_frame.size(), in_frame.type());
	gmm_frame = Mat::zeros(in_frame.size(), CV_8UC1);
	
	gpu_perform_gmm(in_frame.ptr(0), gmm_frame.ptr(0));
	//imshow("GMM", gmm_frame);
	
	performMask(gmm_frame, in_frame, out_frame);
	//imshow("final", out_frame);
}
