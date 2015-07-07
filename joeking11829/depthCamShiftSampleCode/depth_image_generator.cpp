#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;


int getDepthCamera(Mat* image){

	cout << "Depth Device opening ..." << endl;
	
	//image
	Mat depthMap;
	Mat frame;
	
	//Camera ref
	VideoCapture capture;

	capture.open( CAP_OPENNI2 );
	if( !capture.isOpened() )
		capture.open( CAP_OPENNI );


	cout << "done." << endl;

	if( !capture.isOpened() )
	{
		cout << "Can not open a capture object." << endl;
		return -1;
	}


	if( !capture.grab() )
	{
		cout << "Can not grab images." << endl;
		return -1;
	}
	else
	{

		if( capture.retrieve( depthMap, CAP_OPENNI_DEPTH_MAP ) )
		{
			const float scaleFactor = 0.05f;
			Mat show; 
			depthMap.convertTo( show, CV_8UC1, scaleFactor );

			imshow( "depth map", show );

			//Joe add threshold
			threshold(show, show, 30, 255, THRESH_TOZERO);
			threshold(show, show, 45, 255, THRESH_TOZERO_INV);

			adaptiveThreshold(show, show, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

			imshow( "threshold depth map", show );

			//MorphologyEx
			Mat elementA(3, 3, CV_8U, Scalar(1));
			morphologyEx(show, show, cv::MORPH_CLOSE, elementA);

			//findContours
			vector< vector<Point> > contours;
			vector<Vec4i> hierarchy;
			//get Contours external
			findContours(show, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

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

			Mat drawing = Mat::zeros(show.size(), CV_8UC3);
			Mat drawing_line = Mat::zeros(show.size(), CV_8UC3);
			Mat drawing_return = Mat::zeros(show.size(), CV_8UC3);
			Rect shapeRect;
			int area_threshold = 1000;

			for( size_t i = 0; i < contours.size(); i++ )
			{	
				if((contourArea(contours[i]) <= area_threshold) || i != largest_contour_index ){
					continue;
				}

				//Scalar color = Scalar( (rand()&255), (rand()&255), (rand()&255) );
				//drawContours( drawing, contours, int(i), Scalar(255,255,255), CV_FILLED, 8, hierarchy, 0, Point() );
				drawContours( drawing, contours, int(i), Scalar(255,0,0), CV_FILLED, 8, hierarchy, 0, Point() );
				drawContours( drawing_line, contours, int(i), Scalar(255,255,255), 2, 8, hierarchy, 0, Point() );
				shapeRect = boundingRect(contours[i]);
				rectangle(drawing, shapeRect, Scalar(0,255,0), 1, 8, 0);
			}
			//imshow("Contours", drawing);
			//imshow("Contours_Line", drawing_line);
			
			//this image is used by CamShift
			drawing.copyTo(drawing_return);

			//Hull
			vector<vector<Point> > hull( contours.size() );
			vector<vector<int> > hullI( contours.size() );
			vector<vector<Vec4i> > defects( contours.size() );
			for( size_t i = 0; i < contours.size(); i++ )
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
			Mat drawing_hull = Mat::zeros(show.size(), CV_8UC3);
			for( size_t i = 0; i < contours.size(); i++ )
			{

				if((contourArea(contours[i]) <= area_threshold) || i != largest_contour_index){
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
						line(drawing_hull, ptStart, ptFar, CV_RGB(0, 255, 0), 2);
						line(drawing_hull, ptEnd, ptFar, CV_RGB(0, 255, 0), 2);
						circle(drawing_hull, ptStart, 4, Scalar(255, 0, 100), 2);
						circle(drawing_hull, ptEnd, 4, Scalar(255, 0, 100), 2);
						circle(drawing_hull, ptFar, 4, Scalar(255, 0, 100), 2);
					}

					d++;
				}

			}
			imshow("Contours with Circle", drawing);
			imshow("Hull Demo", drawing_hull);

			//Joe End

			//copy image
			drawing_return.copyTo(frame);

		}

		//image set
		frame.copyTo(&image);

	}

	return 0;
}


