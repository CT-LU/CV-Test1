#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;



int getContours(Mat *image, vector< vector<Point> > *contours, vector<Vec4i> *hierarchy){

	//MorphologyEx
	Mat elementA(3, 3, CV_8U, Scalar(1));
	morphologyEx(*image, *image, cv::MORPH_CLOSE, elementA);

	//findContours
	//get Contours external
	findContours(*image, *contours, *hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	return 0;
}

int findLargestContourIndex(vector< vector<Point> > contours){

	//find largest contours
	int largest_area = 0;
	int largest_contour_index = -1;
	for( size_t i = 0; i< contours.size(); i++ ) // iterate through each contour.
	{
		double a = contourArea( contours[i], false);  //  Find the area of contour
		if(a > largest_area){
			largest_area = a;
			largest_contour_index = i;                //Store the index of largest contour
			//bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
		}

	}

	return largest_contour_index;
}

int drawContours(Mat *drawing, vector< vector<Point> > *contours, vector<Vec4i> *hierarchy, int thickness, int largest_contour_index, int area_threshold){
	
	//cout << "contours->size(): " << contours->size() << endl;
	//cout << "largest_contour_index: " << largest_contour_index << endl;
	//cout << "area_threshold: " << area_threshold << endl;
	
	for( size_t i = 0; i < contours->size(); i++ )
	{
	
		//cout << "For Loop: " << i << endl;
		//cout << "contours[i]: " << contourArea((*contours)[i]) << endl;


		if((contourArea((*contours)[i]) <= area_threshold) || i != largest_contour_index ){
			continue;
		}

		drawContours( *drawing, *contours, int(i), Scalar(255,0,0), thickness, 8, *hierarchy, 0, Point() );
	}

	//imshow("drawContours", drawing);

	return 0;
}

/*
int findHullAndDefect(Mat *drawing_hull, vector< vector<Point> > *contours, int largest_contour_index, int area_threshold){

	//Hull
	vector<vector<Point> > hull( contours->size() );
	vector<vector<int> > hullI( contours->size() );
	vector<vector<Vec4i> > defects( contours->size() );
	
	for( size_t i = 0; i < contours->size(); i++ )
	{
		if((contourArea(contours[i]) <= area_threshold) || i != largest_contour_index){
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
	for( size_t i = 0; i < contours->size(); i++ )
	{

		if((contourArea(contours[i]) <= area_threshold) || i != largest_contour_index){
			continue;
		}

		Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
		drawContours( *drawing_hull, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
		
		for(size_t j = 0; j < hull[i].size(); j++)
		{
			color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
			vector<Point> hull_inside = hull[i];
			circle(*drawing_hull, hull_inside[j], 5, color);
		}

		//Draw Defects
		size_t count = contours[i].size();
		//cout << "Count: " << count << endl;

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

			//if( depth > 20 && depth < 150 )
			{
				//show all defect with line
				line(*drawing_hull, ptStart, ptFar, CV_RGB(0, 255, 0), 2);
				line(*drawing_hull, ptEnd, ptFar, CV_RGB(0, 255, 0), 2);
				circle(*drawing_hull, ptStart, 4, Scalar(255, 0, 100), 2);
				circle(*drawing_hull, ptEnd, 4, Scalar(255, 0, 100), 2);
				circle(*drawing_hull, ptFar, 4, Scalar(255, 0, 100), 2);
			}

			d++;
		}

	}

	//imshow("Hull Demo", drawing_hull);

	return 0;
}
*/

Rect getContourBoundingRect(vector<Point> contour){
	
	Rect shapeRect;
	
	//get Rectagle from contour
	shapeRect = boundingRect(contour);
	
	return shapeRect;
}




