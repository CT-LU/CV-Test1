#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <signal.h>
#include <syslog.h>

#include <iostream>
#include <ctype.h>
#include <math.h>

using namespace cv;
using namespace std;


//GMM parameter
const double pi = 3.14159;
const double alpha = 0.00005;
const double def_covariance = 11.0;
const double covariance_threshold = (2.5*2.5);
const double def_weight = 0.00005;
const int max_number_of_gaussian_components = 3;

//Data Structure for GMM
struct gaussian_model
{
	double b_mean;
	double g_mean;
	double r_mean;
	double covariance;
	double weight;
	gaussian_model* next;
	gaussian_model* previous;
};

//Data Structure for Pixel Node
struct pixel_gmm_node
{
	gaussian_model* gaussian_component_start;
	gaussian_model* gaussian_component_rear;
	int number_of_gaussian_components;
	pixel_gmm_node* next;
};

//Global Variable
pixel_gmm_node *pixel_node_start, *pixel_node_rear, *pixel_node_runtime;
gaussian_model *gaussian_start, *gaussian_rear, *gaussian_runtime, *gaussian_tmp_runtime;


//Functions for GMM
pixel_gmm_node* createNodeForPixel(double blue_value, double green_value, double red_value);
void insertNode(pixel_gmm_node* node);
gaussian_model* createGaussianComponentForPixel(double blue_value, double green_value, double red_value);
void insertGaussianComponent(gaussian_model* component);
void deleteGaussianComponent();



//Create Node for each Pixel
pixel_gmm_node* createNodeForPixel(double blue_value, double green_value, double red_value)
{
	pixel_gmm_node* pixel_node = new pixel_gmm_node;

	if(pixel_node != NULL)
	{
		//Node create succeed
		pixel_node->next = NULL;
		pixel_node->number_of_gaussian_components = 1;
		pixel_node->gaussian_component_start = pixel_node->gaussian_component_rear = createGaussianComponentForPixel(blue_value, green_value, red_value);
	}

	return pixel_node;
}


//Create Gaussian Component
gaussian_model* createGaussianComponentForPixel(double blue_value, double green_value, double red_value)
{
	gaussian_model* gaussian_component = new gaussian_model;

	if(gaussian_component != NULL)
	{
		//Gaussian Component create succeed
		gaussian_component->b_mean = blue_value;
		gaussian_component->g_mean = green_value;
		gaussian_component->r_mean = red_value;
		gaussian_component->covariance = def_covariance;
		gaussian_component->weight = def_weight;
		gaussian_component->next = NULL;
		gaussian_component->previous = NULL;
	}

	return gaussian_component;
}

void insertNode(pixel_gmm_node* node)
{
	if(pixel_node_start != NULL)
	{
		pixel_node_rear->next = node;
		pixel_node_rear = node;
	}else{
		pixel_node_start = pixel_node_rear = node;
	}
}

void insertGaussianComponent(gaussian_model* component)
{
	if(gaussian_start != NULL)
	{
		gaussian_rear->next = component;
		component->previous = gaussian_rear;
		gaussian_rear = component;
	}else{
		gaussian_start = gaussian_rear = component;
	}
}

void deleteGaussianComponent()
{
	if(gaussian_rear != NULL)
	{
		gaussian_model* tmp_ptr = gaussian_rear->previous;
		delete gaussian_rear;
		gaussian_rear = tmp_ptr;
	}
}



int main(int argc, char** argv){

	VideoCapture cap;
	Mat frame, gmm_frame;

	/*
	//getDepthCamera
	if(getDepthCamera(&capture) == -1)
	{
	cout << "Open Depth Camera Failed !!" << endl;
	syslog(LOG_ERR, "Open Depth Camera Failed !!");
	return -1;
	}else{
	cout << "Depth Camera Open Succeed !!" << endl;
	syslog(LOG_INFO, "Depth Camera Open Succeed !!");
	}
	 */

	//Open RGB Camera
	cap.open(0);

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

	//Build the first GMM for each pixel
	uchar* row_ptr;
	uchar* output_row_ptr;
	pixel_gmm_node* tmp_node = NULL;


	//Loop parameter
	int i, j, k, sort;
	i = j = k = sort = 0;

	for( i = 0; i < frame.rows; i++ )
	{
		//get buffer for each rows in frame
		row_ptr = frame.ptr(i);

		for( j = 0; j < frame.cols; j++ )
		{
			tmp_node = NULL;
			tmp_node =  createNodeForPixel( *(row_ptr++), *(row_ptr++), *(row_ptr++) );
			if(tmp_node != NULL)
			{	
				//initial weight for first Gaussian component is 1.0
				tmp_node->gaussian_component_rear->weight = 1.0;
				//insert node to Linked List
				insertNode(tmp_node);
			}else{
				cout << "Allocate Memory for Node failed !" << endl;
				exit(0);
			}
		}
	}


	//GMM processing parameter
	int buffer_limited = frame.cols * frame.channels();
	double sum_of_weight = 0.0;
	double weight_runtime = 0.0;
	bool isMatch = false;
	bool isBackground = false;
	double blue_value = 0.0;
	double green_value = 0.0;
	double red_value = 0.0;
	double blue_mean = 0.0;
	double green_mean = 0.0;
	double red_mean = 0.0;
	double blue_diff = 0.0;
	double green_diff = 0.0;
	double red_diff = 0.0;
	double sum_of_square_diff = 0.0;
	double covariance_runtime = 0.0;

	int count_frame = 0;

	for(;;)
	{
		//Get RGB Image
		cap >> frame;
		if( frame.empty() )
		{
			break;
		}

		//debug
		//imshow("Source", frame);

		//GMM output
		gmm_frame = Mat::zeros(frame.size(), CV_8UC1);

		//reset node runtime point
		pixel_node_runtime = pixel_node_start;

		//Matching GMM for each pixel in current frame
		for( i = 0; i < frame.rows; i++ )
		{
			//get buffer for rows in source frame
			row_ptr = frame.ptr(i);
			//get buffer for rows in output frame
			output_row_ptr = gmm_frame.ptr(i);

			//Handle every pixel in one row
			for( j = 0; j < buffer_limited; j += 3)
			{
				//reset parameter
				sum_of_weight = 0.0;
				isMatch = false;
				isBackground = false;

				//get BGR value from each pixel
				blue_value = *(row_ptr++);
				green_value = *(row_ptr++);
				red_value = *(row_ptr++);

				//set GMM point
				gaussian_start = pixel_node_runtime->gaussian_component_start;
				gaussian_rear = pixel_node_runtime->gaussian_component_rear;
				gaussian_runtime = gaussian_start;

				//Macthing current pixel for GMM
				for( k = 0; k < pixel_node_runtime->number_of_gaussian_components; k++  )
				{

					if(!isMatch){
						//Handle matching for each Gaussian Component
						blue_mean = gaussian_runtime->b_mean;
						green_mean = gaussian_runtime->g_mean;
						red_mean = gaussian_runtime->r_mean;

						//get diff
						blue_diff = blue_value - blue_mean;
						green_diff = green_value - green_mean;
						red_diff = red_value - red_mean;

						//get covariance for current gaussian model
						covariance_runtime = gaussian_runtime->covariance;

						//get sum of square diff for BGR
						sum_of_square_diff = (blue_diff*blue_diff + green_diff*green_diff + red_diff*red_diff);

						//judge match or unmatch for current gaussian component
						if( sum_of_square_diff <= (covariance_threshold*covariance_runtime*covariance_runtime) )
						{
							//Match current Gaussian component
							//Update weight
							gaussian_runtime->weight = weight_runtime = (1-alpha)*(gaussian_runtime->weight) + alpha;

							//Update Gaussian Component
							//Update mean
							gaussian_runtime->b_mean = blue_mean = blue_mean + alpha*blue_diff;
							gaussian_runtime->g_mean = green_mean = green_mean + alpha*green_diff;
							gaussian_runtime->r_mean = red_mean = red_mean + alpha*red_diff;

							//get new diff
							blue_diff = blue_value - blue_mean;
							green_diff = green_value - green_mean;
							red_diff = red_value - red_mean;

							//update new sum of square_diff	
							sum_of_square_diff = (blue_diff*blue_diff + green_diff*green_diff + red_diff*red_diff);

							//Update covariance let Rho = alpha
							gaussian_runtime->covariance = covariance_runtime = covariance_runtime + alpha*(sum_of_square_diff - covariance_runtime);

							//Set match flag
							isMatch = true;
							//Set background flag
							isBackground = true;

							//Update data to Match Gaussian Component
							//gaussian_runtime->weight = weight_runtime;
							//gaussian_runtime->b_mean = blue_mean;
							//gaussian_runtime->g_mean = green_mean;
							//gaussian_runtime->r_mean = red_mean;
							//gaussian_runtime->covariance = covariance_runtime;
						}else{
							//UnMatch current Gaussian component
							gaussian_runtime->wieght = weight_runtime = (1-alpha)*(gaussian_runtime->weight);

							//Update data to UnMatch Gaussian Component
							//gaussian_runtime->weight = weight_runtime;
						}

					}else{
						//UnMatch current Gaussian component
						gaussian_runtime->weight = weight_runtime = (1-alpha)*(gaussian_runtime->weight);

						//Update data to UnMatch Gaussian Component
						//gaussian_runtime->weight = weight_runtime;
					}

					//get sum of weight
					sum_of_weight += weight_runtime;

					//matching next Gaussian component
					gaussian_runtime = gaussian_runtime->next;
				}

				//if there is no match in GMM, delete the least weight gaussian component
				if(!isMatch)
				{
					//Create new Gaussian component
					gaussian_runtime = createGaussianComponentForPixel(blue_value, green_value, red_value);
					/*
					   gaussian_runtime = new gaussian_model;
					   gaussian_runtime->weight = def_weight;
					   gaussian_runtime->b_mean = blue_value;
					   gaussian_runtime->g_mean = green_value;
					   gaussian_runtime->r_mean = red_value;
					   gaussian_runtime->covariance = def_covariance;
					   gaussian_runtime->next = NULL;
					   gaussian_runtime->previous = NULL;
					 */

					//Delete the least weight of Gaussian component and Add the new Gaussian component to GMM
					//Delete the least weight of Gaussian component if number of Gaussian components is not reach to the maximun number
					if(pixel_node_runtime->number_of_gaussian_components <= max_number_of_gaussian_components)
					{
						//Not reach the Maxmium yet.
						//add new Gaussian component to current pixel's GMM
						insertGaussianComponent(gaussian_runtime);

						//increase number of gaussian component
						pixel_node_runtime->number_of_gaussian_components += 1;
						//update pixel_node_runtime information
						pixel_node_runtime->gaussian_component_rear = gaussian_rear;

						//record the new sum of weight
						sum_of_weight += gaussian_runtime->weight;

					}else{
						//Reach the Maxmium
						//delete the least weight Gaussian component and add new Gaussian component to current pixel's GMM
						sum_of_weight -= gaussian_rear->weight;
						deleteGaussianComponent();

						//add the new Gaussian component
						insertGaussianComponent(gaussian_runtime);

						//update pixel_node_runtime information
						pixel_node_runtime->gaussian_component_rear = gaussian_rear;

						//increase number of gaussian component
						//pixel_node_runtime->number_of_gaussian_components++;

						//record the new sum of weight
						sum_of_weight += gaussian_runtime->weight;
					}
				}


				//normalize the sum of weight to 1, if sum of weight < 0.9 or sum of weight > 1.2
				//do normalization
				if(sum_of_weight < 0.9 || sum_of_weight > 1.2)
				{
					//Normalize the weight for each Gaussian component
					gaussian_runtime = gaussian_start;
					while( gaussian_runtime != NULL )
					{
						gaussian_runtime->weight /= sum_of_weight;
						gaussian_runtime = gaussian_runtime->next;
					}
				}

				//reorder Gaussian components for current pixel's GMM
				//reorder by weight/covariance with Bubble sort

				//cout << "Current Number of Gaussian Components:" << pixel_node_runtime->number_of_gaussian_components << endl;
				/*
				for( sort = 0; sort < pixel_node_runtime->number_of_gaussian_components; sort++)
				{
					gaussian_runtime = gaussian_start;
					while(gaussian_runtime != NULL)
					{
						if(gaussian_runtime->next != NULL)
						{
							//Not the end of linked list
							double weight_01 = (gaussian_runtime->weight)/(gaussian_runtime->covariance);
							double weight_02 = (gaussian_runtime->next->weight)/(gaussian_runtime->next->covariance);

							if(weight_01 < weight_02){
								//change order
								gaussian_tmp_runtime = gaussian_runtime->next;

								//Linked List exchange !!
								if(gaussian_runtime->next == gaussian_rear){
									//the last gaussian component need to be exchanged
									gaussian_rear = gaussian_runtime;
									pixel_node_runtime->gaussian_component_rear = gaussian_rear;

									gaussian_runtime->next = NULL;
								}else{
									gaussian_runtime->next = gaussian_tmp_runtime->next;
									gaussian_runtime->next->previous = gaussian_runtime;
								}
								gaussian_tmp_runtime->next = gaussian_runtime;

								if(gaussian_runtime == gaussian_start){
									//the first gaussian component need to be exchanged
									gaussian_start = gaussian_tmp_runtime;
									pixel_node_runtime->gaussian_component_start = gaussian_start;

									gaussian_tmp_runtime->previous = NULL;
								}else{
									gaussian_tmp_runtime->previous = gaussian_runtime->previous;
									gaussian_tmp_runtime->previous->next = gaussian_tmp_runtime;
								}

								gaussian_runtime->previous = gaussian_tmp_runtime;
								gaussian_runtime = gaussian_tmp_runtime;

							}

						}
						//next loop
						gaussian_runtime = gaussian_runtime->next;
					}
				}
				*/

				//update pixel_node_runtime information
				//pixel_node_runtime->gaussian_component_start = gaussian_start;
				//pixel_node_runtime->gaussian_component_rear = gaussian_rear;

				//judge foreground or background for current pixel and set the result to the gmm_frame
				if(isBackground){
					//Match one of background GMM
					//*(output_row_ptr++) = ((int) 0);
					output_row_ptr++;
				}else{
					//UnMatch any one of background GMM
					*(output_row_ptr++) = ((int) 255);
				}

				//move the GMM to next pixel node
				pixel_node_runtime = pixel_node_runtime->next;

			}
			//LOOP: go to next row
		}

		count_frame++;
		//cout << "Number of Frame is :" << count_frame << endl;

		//Show the GMM result image
		imshow("GMM", gmm_frame);

		//User Key Input
		waitKey(30);
	}

	return 0;
}

