#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;
using namespace cv;

//source of input image
Mat image;

//store result of findContours
vector< vector<Point> > contours;
vector<Vec4i> hierarchy;

//Mat for result image
Mat drawing;
Mat drawing_hull;

//filter small area
int area_threshold = 100;


void colorSpaceHandler();
void shapeHandler();
void edgeDetector();
void contoursHandler();
void findHullAndDefect();
float distanceP2P(Point a, Point b);


int main(int argc, char** argv)
{
	//Load image
	image = imread(argv[1]);
	if( image.empty() )
	{
		cerr << "No image supplied ..." << endl;
		return -1;
	}

	//initial Mat
	drawing = Mat::zeros(image.size(), CV_8UC3);
	drawing_hull = Mat::zeros(image.size(), CV_8UC3);

	//get Gray image
	colorSpaceHandler();

	//Let the shape to be full
	shapeHandler();

	//find Edge
	edgeDetector();

	//find Contours
	contoursHandler();

	//get Hull and Defect
	findHullAndDefect();

	//keyCode return
	waitKey(0);

	/*
	   if(keycode == 27){
	   break;
	   }
	 */

	return 0;
}


void colorSpaceHandler()
{
	//Color to Gray
	cvtColor(image, image, CV_BGR2GRAY);
	imshow("Gray Image", image);
}


void shapeHandler()
{
	//Dilate
	//Mat elementA(3, 3, CV_8U, Scalar(1));
	//dilate(image, image, elementA);
	//imshow("Dilate", image);

	//Erode
	//Mat elementB(3, 3, CV_8U, Scalar(1));
	//erode(image, image, elementB, Point(-1, -1), 3);
	//imshow("Erode", image);

	//MorphologyEx
	Mat elementC(3, 3, CV_8U, Scalar(1));
	morphologyEx(image, image, cv::MORPH_CLOSE, elementC);
	imshow("Morph_close Image", image);
}


void edgeDetector()
{
	//Canny Edge Detector
	int canny_thresh = 50;
	Canny( image, image, canny_thresh, canny_thresh*4, 5);
	imshow("Canny", image);
}


void contoursHandler()
{
	//findContours
	findContours(image, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));
	//findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	//Mat drawing_lv1 = Mat::zeros(image.size(), CV_8UC3);
	//Mat drawing_lv2 = Mat::zeros(image.size(), CV_8UC3);

	for( size_t i = 0; i < contours.size(); i++ )
	{
		if(contourArea(contours[i]) <= area_threshold){
			continue;
		}

		Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
		drawContours( drawing, contours, int(i), color, 2, 8, hierarchy, 0, Point() );
		//drawContours( drawing_lv1, contours, int(i), color, 2, 8, hierarchy, 1, Point() );
		//drawContours( drawing_lv2, contours, int(i), color, 2, 8, hierarchy, 2, Point() );

	}
	imshow("Pure_Contours", drawing);
	//imshow("Pure_Contours_lv1", drawing_lv1);
	//imshow("Pure_Contours_lv2", drawing_lv2);

}


float distanceP2P(Point a, Point b)
{
	float d = sqrt(fabs( pow( a.x-b.x, 2) + pow( a.y-b.y, 2) ));
	//cout << "Distance: " << d << endl;
	return d;
}


void findHullAndDefect()
{
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
		
		
		cout << "Origin hull[" << i << "]" << endl;
		for(size_t aa = 0; aa < hull[i].size(); aa++){
			cout << "hull[" << i << "][" << aa << "]: " << hull[i][aa] << endl;
		}
		
		/*
		//reduce Hull
		vector<Point> tmp_hull = hull[i];
		vector<int> tmp_hullI = hullI[i];
		vector<Point> reduce_hull;
		vector<int> reduce_hullI;

		//compare all point
		for(size_t j = 0; j < tmp_hull.size(); j++){	
				cout << " j = " << j << endl;		
				if(tmp_hull[j].x == -1 && tmp_hull[j].y == -1){
					continue;
				}

			for(size_t k = j; k < tmp_hull.size(); k++){
				cout << " k = " << k << endl;
				if(tmp_hull[k].x == -1 && tmp_hull[k].y == -1){
					continue;
				}

				float d = distanceP2P(tmp_hull[j], tmp_hull[k]);
				cout << "tmp_hull[" << j << "]: " << tmp_hull[j] << " and tmp_hull[" << k << "]: " << tmp_hull[k] <<  " -> Distance is: " << d << endl;
				if( d < 15 && j != k ){
					tmp_hull[k] = Point(-1, -1);
					cout << "Set tmp_hull[" << k << "] = Point(-1,-1) " << endl;
				}else{
					cout << "Push tmp_hull[" << k << "] to reduce_hull" << endl;
					if(j == k){
						reduce_hull.push_back(tmp_hull[k]);
						reduce_hullI.push_back(tmp_hullI[k]);
					}
					//break;
				}
			}
		}

		cout << "Changed hull[" << i << "]" << endl;
		for(size_t aa = 0; aa < reduce_hull.size(); aa++){
			cout << "reduce_hull[" << aa << "]: " << reduce_hull[aa] << endl;
		}



		hull[i].swap(reduce_hull);
		hullI[i].swap(reduce_hullI);
		*/


		//approxPolyDP( Mat(contours[i]), hull[i], 18, true );  //Draw PolyDP
		if(contours[i].size() > 3){
			convexityDefects( Mat(contours[i]), hullI[i], defects[i] );
		}
	}

	//Drawing Hull
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
		cout << "Count: " << count << endl;

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

			if(  depth < 100 )
			{
				//show all defect with line
				line(drawing_hull, ptStart, ptFar, CV_RGB(0, 255, 0), 2);
				line(drawing_hull, ptEnd, ptFar, CV_RGB(0, 255, 0), 2);
				circle(drawing_hull, ptStart, 4, Scalar(255, 0, 100), 2);
				circle(drawing_hull, ptEnd, 4, Scalar(255, 0, 100), 2);
				circle(drawing_hull, ptFar, 4, Scalar(255, 0, 100), 2);
				circle(drawing, ptFar, 4, Scalar(255, 0, 100), 2);
			}

			d++;
		}

	}
	imshow("Contours", drawing);
	imshow("Hull Demo", drawing_hull);

}


