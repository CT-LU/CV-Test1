#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;


int getContours(Mat *image, vector< vector<Point> > *contours, vector<Vec4i> *hierarchy);

int findLargestContourIndex(vector< vector<Point> > contours);

int drawContours(Mat *drawing, vector< vector<Point> > *contours, vector<Vec4i> *hierarchy, int thickness, int largest_contour_index, int area_threshold);

//int findHullAndDefect(Mat *drawing_hull, vector< vector<Point> > *contours, int largest_contour_index, int area_threshold);

Rect getContourBoundingRect(vector<Point> contour);

