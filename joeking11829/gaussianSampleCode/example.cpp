#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string>

using namespace std;
using namespace cv;

static void help()
{
	printf("\n"
			"This program demonstrated a simple method of connected components clean up of background subtraction\n"
			"When the program starts, it begins learning the background.\n"
			"You can toggle background learning on and off by hitting the space bar.\n"
			"Call\n"
			"./segment_objects [video file, else it reads camera 0]\n\n");
}


int main(int argc, char** argv)
{
	VideoCapture cap;
	bool update_bg_model = true;

	help();

	if( argc < 2 )
		cap.open(0);
	else
		cap.open(std::string(argv[1]));

	if( !cap.isOpened() )
	{
		printf("\nCan not open camera or video file\n");
		return -1;
	}

	Mat tmp_frame, bgmask, out_frame;

	cap >> tmp_frame;
	if(tmp_frame.empty())
	{
		printf("can not read data from the video source\n");
		return -1;
	}

	namedWindow("video", 1);
	namedWindow("segmented", 1);
	//namedWindow("controls", WINDOW_AUTOSIZE);

	Ptr<BackgroundSubtractorMOG2> bgsubtractor=createBackgroundSubtractorMOG2();
	bgsubtractor->setVarThreshold(10);
	bgsubtractor->setNMixtures(3);
	bgsubtractor->setDetectShadows(false);
	bgsubtractor->setBackgroundRatio(1.0);
	//cout << bgsubtractor->getBackgroundRatio() << endl;

	int thresh = 100;
	int max_thresh = 255;
	int canny_thresh = 50;
	int static_thresh = 50;
	int ludanCounter = 0;

	//createTrackbar("Binary thresh", "controls", &static_thresh, max_thresh);
	//createTrackbar("Canny thresh", "controls", &canny_thresh, max_thresh);

	for(;;)
	{
		cap >> tmp_frame;
		if( tmp_frame.empty() )
			break;


		//learning model
		//bgsubtractor->apply(tmp_frame, bgmask, update_bg_model ? -1 : 0);
		bgsubtractor->apply(tmp_frame, bgmask, ludanCounter ? 0 : -1);

		//imshow("video", tmp_frame);
		imshow("segmented", bgmask);
		//imshow("segmented", out_frame);

		//Joe

		//Erode
		Mat elementA(2, 2, CV_8U, Scalar(1));
		erode(bgmask, bgmask, elementA, Point(-1, -1), 1);
		imshow("Erode", bgmask);

		//Dilate
		Mat elementB(3, 3, CV_8U, Scalar(1));
		dilate(bgmask, bgmask, elementB, Point(-1,-1), 4);
		imshow("Dilate", bgmask);

		//MorphologyEx
		Mat elementC(1, 1, CV_8U, Scalar(1));
		morphologyEx(bgmask, bgmask, cv::MORPH_CLOSE, elementC);
		imshow("morph_close", bgmask);


		//Canny
		//Canny(bgmask, bgmask, canny_thresh, canny_thresh*4, 5);
		//imshow("Canny_image", bgmask);

		//findContours
		vector< vector<Point> > contours;
		vector<Vec4i> hierarchy;
		//get Contours external
		findContours(bgmask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0) );

		int area_threshold = 1000;
		//Mat result(bgmask.size(), CV_8U, Scalar(255));
		Mat drawing = Mat::zeros(bgmask.size(), CV_8UC3);
		for( size_t i = 0; i < contours.size(); i++ )
		{

			if(contourArea(contours[i]) <= area_threshold){
				continue;
			}


			Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
			drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
			drawContours( tmp_frame, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
		}

		//Hull
		vector<vector<Point> > hull( contours.size() );
		vector<vector<int> > hullI( contours.size() );
		vector<vector<Vec4i> > defects( contours.size() );
		for( size_t i = 0; i < contours.size(); i++ )
		{
			if(contourArea(contours[i]) <= area_threshold){
				continue;
			}

			convexHull( Mat(contours[i]), hull[i], false, true );
			convexHull( Mat(contours[i]), hullI[i], false, false );
			//approxPolyDP( Mat(contours[i]), hull[i], 18, true );  //Draw PolyDP
			if(contours[i].size() > 3){
				convexityDefects( Mat(contours[i]), hullI[i], defects[i] );
			}
		}

		//Drawing Hull
		Mat drawing_hull = Mat::zeros(bgmask.size(), CV_8UC3);
		for( size_t i = 0; i < contours.size(); i++ )
		{

			if(contourArea(contours[i]) <= area_threshold){
				continue;
			}

			Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
			drawContours( drawing_hull, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
			drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
			for(size_t j = 0; j < hull[i].size(); j++)
			{
				color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
				vector<Point> hull_inside = hull[i];
				circle(drawing, hull_inside[j], 5, color);
				circle(drawing_hull, hull_inside[j], 5, color);
			}

			//Draw Defects
			size_t count = contours[i].size();
		//	cout << "Count: " << count << endl;

			if( count < 30 ){
				continue;
			}

			vector<Vec4i>::iterator d = defects[i].begin();

			while( d != defects[i].end() )
			{
				Vec4i& v = (*d);

				// Index of defect vector
				int startIdx = v[0];
				int endIdx = v[1];
				int farIdx = v[2];

				//Point of the contour where the defect begins
				Point ptStart( contours[i][startIdx] );

				//Point of the contour where the defect ends
				Point ptEnd( contours[i][endIdx] );

				//the farthest from the convex hull point within the defect
				Point ptFar( contours[i][farIdx] );

				//distance between the farthest point and the convex hull
				int depth = v[3] / 256;

				if( depth > 20 && depth < 150 )
				{
					//show all defect with line
					line(drawing_hull, ptStart, ptFar, CV_RGB(0, 255, 0), 2);
					line(drawing_hull, ptEnd, ptFar, CV_RGB(0, 255, 0), 2);
					circle(drawing_hull, ptStart, 4, Scalar(255, 0, 100), 2);
					circle(drawing_hull, ptEnd, 4, Scalar(255, 0, 100), 2);
					circle(drawing_hull, ptFar, 4, Scalar(255, 0, 100), 2);
				}

				d++;
			}

		}
		imshow("Contours", drawing);
		imshow("Hull Demo", drawing_hull);



		int keycode = waitKey(20);
		if( keycode == 27 )
			break;
			
		//cout << ludanCounter << endl;
		if( ludanCounter == 3 )
		//if( keycode == ' ' )
		{
			update_bg_model = !update_bg_model;
			printf("Learn background is in state = %d\n",update_bg_model);
			ludanCounter = 0;
		}
		else ludanCounter++;
	}

	return 0;
}
