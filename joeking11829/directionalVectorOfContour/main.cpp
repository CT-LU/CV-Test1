#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "depth_image_generator.hpp"
#include "contours_operator.hpp"


#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;


float distanceP2P(Point a, Point b)
{
	float d = sqrt(fabs( pow( a.x-b.x, 2) + pow( a.y-b.y, 2) ));
	//cout << "Distance: " << d << endl;
	return d;
}


int main(int argc, char** argv){

	VideoCapture capture;
	Mat image, gray_image, color_image, drawing, gray_drawing, color_drawing, depth_drawing, color_depth_drawing, mask;

	//getDepthCamera
	if(getDepthCamera(&capture) == -1)
	{
		cout << "Open Depth Camera Failed !!" << endl;
		return -1;
	}else{
		cout << "Depth Camera Open Succeed !!" << endl;
	}

	//Calculate ROI parameter
	int source_image_width = 640;                                                                                                                                         
	int source_image_height = 480;

	//ROI Rect
	int roi_rect_width = source_image_width * 0.6;
	int roi_rect_height = source_image_height * 0.5;
	int roi_rect_start_x = (source_image_width - roi_rect_width) * 0.5 ;
	int roi_rect_start_y = (source_image_height - roi_rect_height) * 0.5;

	//ROI for tracking
	Rect roi_rect(roi_rect_start_x, roi_rect_start_y, roi_rect_width, roi_rect_height);

	//condition
	int start_depth = 30;
	int end_depth = 60;
	int area_threshold = 100;



	for(;;)
	{
		//getDepthImage
		//getDepthImage(capture, &image);

		//getColorDepthImage
		getDepthImage(capture, &image, &color_image);

		imshow("Gray Depth Source", image);
		//imshow("Color Depth Source", color_image);

		//CopyTo gray_image
		image.copyTo(gray_image);

		//filterDepthImage
		filterDepthImage(&image, &gray_image, &image, start_depth, end_depth);

		//show depth image
		//imshow("Main Program 2 for Depth Camera", gray_image);
		//imshow("Main Program for Depth Camera", image);

		//Create ROI Mask
		mask = Mat(image.size(), CV_8UC1, Scalar::all(255));
		mask(roi_rect).setTo(Scalar::all(0));
		//imshow("Mask", mask);

		image.setTo( 0, mask);
		gray_image.setTo( 0, mask);
		//imshow("Main Program for Depth Camera with Mask", image);


		//handle Contours
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		//findContours
		getContours(&image, &contours, &hierarchy);

		//get largest contour
		int largest_contour_index = findLargestContourIndex(contours);
		//cout << "largest_contour_index: " << largest_contour_index << endl;

		//draw Contours
		drawing = Mat::zeros(image.size(), CV_8UC3);
		color_drawing = Mat::zeros(image.size(), CV_8UC3);
		gray_drawing = Mat::zeros(image.size(), CV_8UC1);
		depth_drawing = Mat::zeros(image.size(), CV_8UC1);
		color_depth_drawing = Mat::zeros(image.size(), CV_8UC3);

		//Using Contour tracking Object
		if(largest_contour_index != -1)
		{
			//depth image exist
			//drawContours(&drawing, &contours, &hierarchy, -1, largest_contour_index, area_threshold);

			mask = Mat(image.size(), CV_8U, Scalar(0));

			drawContours(mask, contours, largest_contour_index, Scalar(255), 1);

			//fine largest depth value
			vector<Point> largest_contour = contours[largest_contour_index];

			//out << "vector<Point>: " << largest_contour << endl << endl;

			/*
			   for(size_t i = 0; i < largest_contour.size(); i++)
			   {
			   Point dp = largest_contour[i];
			//cout << "x:" << dp.x << " y:" << dp.y << " depth:" << gray_image.at<Point>(dp) << endl;

			cout << "Point dp:" << dp << " pixel:" << ((int)gray_image.at<uchar>(dp)) << endl;
			}
			 */


			//get Gray and Color ROI
			color_image.copyTo(color_drawing, mask);
			gray_image.copyTo(gray_drawing, mask);
			//copy gray to depth
			gray_drawing.copyTo(depth_drawing);

			int max_pixel = 0;
			int min_pixel = 255;
			for(size_t i = 0; i < gray_drawing.rows; i++)
			{
				for(size_t j = 0; j < gray_drawing.cols; j++)
				{
					int pval = (int)(gray_drawing.at<uchar>(i,j));

					if(pval > max_pixel)
					{
						max_pixel = pval;
					}
					if(pval < min_pixel && pval != 0)
					{
						min_pixel = pval;
					}
				}
			}
			//cout << "Max_Pixel:" << max_pixel << " Min_Pixel:" << min_pixel << endl << endl;

			//Histogram parameter
			Mat gray_hist;
			int histSize = max_pixel - min_pixel + 1;
			float range[] = { 0, 255 };
			const float* histRange = range;

			//calculate histogram of gray image
			calcHist( &gray_drawing, 1, 0, mask, gray_hist, 1, &histSize, &histRange );


			int hist_w = 500;
			int hist_h = 500;
			int bin_w = cvRound( (double) hist_w/histSize );

			Mat histImage( hist_h, hist_w, CV_8UC3, Scalar(0,0,0) );

			normalize(gray_hist, gray_hist, 0, histImage.rows, NORM_MINMAX);

			for( int i = 0; i < histSize; i++ )
			{
				int val = saturate_cast<int>(gray_hist.at<float>(i)*histImage.rows/255);

				rectangle( histImage, 
						Point( bin_w*(i), histImage.rows ),
						Point( bin_w*(i+1), histImage.rows - val ),
						Scalar(181, 186, 10), -1, 8 );
			}

			imshow("Histogram of User's arm", histImage);


			//find points set in each depth
			//get start depth and end depth
			int start_depth = 0;
			int end_depth = 0;
			if( (max_pixel - min_pixel) >= 7 )
			{
				start_depth = min_pixel + 5;
				end_depth = max_pixel - 2;
			}else{
				start_depth = min_pixel;
				end_depth = max_pixel;
			}
			Point compare = Point(-1,-1);

			//cout << "start_depth: " << start_depth << endl;
			//cout << "end_depth: " << end_depth << endl;

			for(size_t depth = start_depth; depth < (end_depth+1); depth++)
			{

				//paramet for points set
				int side_a_count = 0;
				int side_a_x_total = 0;
				int side_a_y_total = 0;
				int side_b_count = 0;
				int side_b_x_total = 0;
				int side_b_y_total = 0;
				compare = Point(-1,-1);
				
				//select Color
				Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );

				//handle each depth in a for loop
				for(size_t count = 0; count < largest_contour.size(); count++)
				{
					Point dp = largest_contour[count];

					//get point depth information
					int real_depth = (int)(gray_drawing.at<uchar>(dp));

					if(real_depth == depth)
					{
						//draw depth point
						circle(color_depth_drawing, dp, 2, color, 2);

						//find compare point
						if(compare == Point(-1,-1))
						{
							//crete first compare point
							compare = dp;
							side_a_count++;
							side_a_x_total += compare.x;
							side_a_y_total += compare.y;
						}else
						{
							int distance = (int)(distanceP2P(compare, dp));
							if(distance <= 20)
							{
								side_a_count++;
								side_a_x_total += dp.x;
								side_a_y_total += dp.y;
							}else
							{
								side_b_count++;
								side_b_x_total += dp.x;
								side_b_y_total += dp.y;

							}
						}
					}

				}

				//find mean point
				if(side_a_count > 0 && side_b_count > 0)
				{
					//mean point of side_a
					Point side_a_mean = Point(-1,-1);
					side_a_mean.x = (int)(side_a_x_total/side_a_count);
					side_a_mean.y = (int)(side_a_y_total/side_a_count);

					//mean point of side_b
					Point side_b_mean = Point(-1,-1);
					side_b_mean.x = (int)(side_b_x_total/side_b_count);
					side_b_mean.y = (int)(side_b_y_total/side_b_count);

					//mean point of this depth
					Point depth_mean = Point(-1,-1);
					depth_mean.x = (int)((side_a_mean.x + side_b_mean.x)/2);
					depth_mean.y = (int)((side_a_mean.y + side_b_mean.y)/2);

					//draw this point
					circle(depth_drawing, depth_mean, 3, color, 2);
					circle(color_depth_drawing, depth_mean, 3, color, 2);
					circle(color_drawing, depth_mean, 3, color, 2);
				}

			}
		}

		//Draw ROI
		//rectangle( drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		rectangle( gray_drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		rectangle( color_drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		rectangle( depth_drawing, roi_rect, Scalar(181,186,10), 2, 8 );
		rectangle( color_depth_drawing, roi_rect, Scalar(181,186,10), 2, 8 );

		//imshow("Contour tracking Object", drawing);
		//Show Gray and Color Image
		imshow("Color ROI Drawing", color_drawing);
		imshow("Gray ROI Drawing", gray_drawing);
		imshow("Depth ROI Drawing", depth_drawing);
		imshow("Color Depth ROI Drawing", color_depth_drawing);



		waitKey(30);
	}

	return 0;
}

