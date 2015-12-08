#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <signal.h>
#include <syslog.h>
#include <errno.h>

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;


//GMM parameter
const float pi = 3.14159;
const float alpha = 0.00005;
const float def_covariance = 11.0;
const float covariance_threshold = (2.5*2.5);
const float def_weight = 0.00005;
const int max_number_of_gaussian_components = 3;

//Data Structure for GMM
struct gaussian_model
{
	float b_mean;
	float g_mean;
	float r_mean;
	float covariance;
	float weight;
};

int main(int argc, char** argv)
{
	VideoCapture cap;
	Mat frame;
	frame.create(Size(1280,720), CV_8UC1);
	
	if ( frame.isContinuous() ) cout << "yes" << endl;
	//Open RGB Camera
	cap.open(0);
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

	if( !cap.isOpened() )
	{
		cout << "Can not open camera !!" << endl;
		return -1;
	}

	//read frame
	cap >> frame;
	if( frame.empty() )
	{
		cout << "Can not read data from the Camera !!" << endl;
		return -1;
	}
	
	gaussian_model* models = new gaussian_model[frame.cols * frame.rows * max_number_of_gaussian_components]; 
	cout << "frame.cols: " << frame.cols << endl;
	cout << "frame.rows: " << frame.rows << endl;

	//Build the first GMM for each pixel
	uchar* row_ptr;
	uchar* output_row_ptr;

	for( int i = 0; i < frame.rows; i++ )
	{
		//get buffer for each rows in frame
		row_ptr = frame.ptr(i);

		for( int j = 0; j < frame.cols; j++ )
		{
			int index = ((i * frame.cols) + j) * max_number_of_gaussian_components;
			
			models[index].b_mean = *(row_ptr++);
			models[index].g_mean = *(row_ptr++);
			models[index].r_mean = *(row_ptr++);
			models[index].covariance = def_covariance;
			models[index].weight = 1.0;
			for ( int k = 1; k <= max_number_of_gaussian_components; k++ ) {
				models[index + k].weight = 0.0;
			}
		}
	}

	//GMM processing parameter
	float sum_of_weight = 0.0;
	float weight_runtime = 0.0;
	bool isMatch = false;
	bool isBackground = false;
	float blue_value = 0.0;
	float green_value = 0.0;
	float red_value = 0.0;
	float blue_mean = 0.0;
	float green_mean = 0.0;
	float red_mean = 0.0;
	float blue_diff = 0.0;
	float green_diff = 0.0;
	float red_diff = 0.0;
	float sum_of_square_diff = 0.0;
	float covariance_runtime = 0.0;


	for(;;)
	{
		//Get RGB Image
		cap >> frame;
		if( frame.empty() )
			break;

		//GMM output
		Mat gmm_frame;
		gmm_frame.create(frame.size(), frame.type());
		gmm_frame = Mat::zeros(frame.size(), CV_8UC1);

		//reset node runtime point
		gaussian_model* current_model = NULL;

		//Matching GMM for each pixel in current frame
		for( int i = 0; i < frame.rows; i++ )
		{
			//get buffer for rows in source frame
			row_ptr = frame.ptr(i);
			output_row_ptr = gmm_frame.ptr(i);

			//Handle every pixel in one row
			for( int j = 0; j < frame.cols; j++) {

				current_model = &models[((i * frame.cols) + j) * max_number_of_gaussian_components]; 
				
				//reset parameter
				sum_of_weight = 0.0;
				isMatch = false;
				isBackground = false;

				//get BGR value from each pixel
				blue_value = *(row_ptr++);
				green_value = *(row_ptr++);
				red_value = *(row_ptr++);

				//Macthing current pixel for GMM
				for(int k = 0; k < max_number_of_gaussian_components; k++) {

					if (current_model->weight == 0) {
						current_model++;
						continue;
					}

					if(!isMatch) {
						//Handle matching for each Gaussian Component
						blue_mean = current_model->b_mean;
						green_mean = current_model->g_mean;
						red_mean = current_model->r_mean;

						//get diff
						blue_diff = blue_value - blue_mean;
						green_diff = green_value - green_mean;
						red_diff = red_value - red_mean;

						//get covariance for current gaussian model
						covariance_runtime = current_model->covariance;

						//get sum of square diff for BGR
						sum_of_square_diff = (blue_diff*blue_diff + green_diff*green_diff + red_diff*red_diff);

						//judge match or unmatch for current gaussian component
						if( sum_of_square_diff <= (covariance_threshold*covariance_runtime*covariance_runtime) )
						{
							//Match current Gaussian component
							//Update weight
							current_model->weight = weight_runtime = (1-alpha)*(current_model->weight) + alpha;

							//Update Gaussian Component
							//Update mean
							current_model->b_mean = blue_mean = blue_mean + alpha*blue_diff;
							current_model->g_mean = green_mean = green_mean + alpha*green_diff;
							current_model->r_mean = red_mean = red_mean + alpha*red_diff;

							//get new diff
							blue_diff = blue_value - blue_mean;
							green_diff = green_value - green_mean;
							red_diff = red_value - red_mean;

							//update new sum of square_diff	
							sum_of_square_diff = (blue_diff*blue_diff + green_diff*green_diff + red_diff*red_diff);

							//Update covariance let Rho = alpha
							current_model->covariance = covariance_runtime = covariance_runtime + alpha*(sum_of_square_diff - covariance_runtime);

							//Set match flag
							isMatch = true;
							//Set background flag
							isBackground = true;

						} else {
							//UnMatch current Gaussian component
							current_model->weight = weight_runtime = (1-alpha)*(current_model->weight);
						}
					} else {
						//UnMatch current Gaussian component
						current_model->weight = weight_runtime = (1-alpha)*(current_model->weight);
					}

					//get sum of weight
					sum_of_weight += weight_runtime;

					//matching next Gaussian component
					current_model++;
				} // the end of the k components

				//if there is no match in GMM, delete the least weight gaussian component
				if(!isMatch) {

					current_model = &models[((i * frame.cols) + j) * max_number_of_gaussian_components]; 
					float min_weight = current_model->weight;
					int min_component = 0;
					
					for (int gg = 1; gg <= max_number_of_gaussian_components; gg++) {
						if (current_model[gg].weight < min_weight) {
							min_weight = current_model[gg].weight;
							min_component = gg;
						}
					}					

					current_model += min_component;
					sum_of_weight -= current_model->weight;

					current_model->b_mean = blue_value;
					current_model->g_mean = green_value;
					current_model->r_mean = red_value;
					current_model->covariance = def_covariance;
					current_model->weight = def_weight;
					
					//record the new sum of weight
					sum_of_weight += current_model->weight;
				}

				current_model = &models[((i * frame.cols) + j) * max_number_of_gaussian_components]; 
				//normalize the sum of weight to 1, if sum of weight < 0.9 or sum of weight > 1.2
				//do normalization
				if(sum_of_weight < 0.9 || sum_of_weight > 1.2)
				{
					//Normalize the weight for each Gaussian component
					for (int gg = 0; gg < max_number_of_gaussian_components; gg++) {
						current_model += gg;
						current_model->weight /= sum_of_weight;
					}
				}

				//judge foreground or background for current pixel and set the result to the gmm_frame
				if(isBackground){
					//Match one of background GMM
					output_row_ptr++;
				}else{
					//UnMatch any one of background GMM
					*(output_row_ptr++) = ((int) 255);
				}
			}
			//LOOP: go to next row
		}

		//Show the GMM result image
		//imshow("GMM", frame);
		imshow("GMM", gmm_frame);

		//User Key Input
		waitKey(30);
	}

	delete[] models;

	return 0;
}

