#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

Mat image;
bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;

/* Marked by Adi
 *
 * Point(int x, int y)
 *
 * x: x-coordinate of the point
 * y: y-coordinate of the point
 *
 */


/* Marked by Adi
 *
 * Rect (int x, int y, int width, int height)
 *
 * x: x-coordinate of the top-left corner
 * y: y-coordinate of the top-left corner (sometimes bottom-left corner)
 * width: width of the rectangle
 * height: height of the rectangle
 *
 */


int vmin = 10, vmax = 256, smin = 30;




static void colorizeDisparity( const Mat& gray, Mat& rgb, double maxDisp=-1.f, float S=1.f, float V=1.f )
{
    CV_Assert( !gray.empty() );
    CV_Assert( gray.type() == CV_8UC1 );

    if( maxDisp <= 0 )
    {
	maxDisp = 0;
	minMaxLoc( gray, 0, &maxDisp );
    }

    rgb.create( gray.size(), CV_8UC3 );
    rgb = Scalar::all(0);
    if( maxDisp < 1 )
	return;

    for( int y = 0; y < gray.rows; y++ )
    {
	for( int x = 0; x < gray.cols; x++ )
	{
	    uchar d = gray.at<uchar>(y,x);
	    unsigned int H = ((uchar)maxDisp - d) * 240 / (uchar)maxDisp;

	    unsigned int hi = (H/60) % 6;
	    float f = H/60.f - H/60;
	    float p = V * (1 - S);
	    float q = V * (1 - f * S);
	    float t = V * (1 - (1 - f) * S);

	    Point3f res;

	    if( hi == 0 ) //R = V,  G = t,  B = p
		res = Point3f( p, t, V );
	    if( hi == 1 ) // R = q, G = V,  B = p
		res = Point3f( p, V, q );
	    if( hi == 2 ) // R = p, G = V,  B = t
		res = Point3f( t, V, p );
	    if( hi == 3 ) // R = p, G = q,  B = V
		res = Point3f( V, q, p );
	    if( hi == 4 ) // R = t, G = p,  B = V
		res = Point3f( V, p, t );
	    if( hi == 5 ) // R = V, G = p,  B = q
		res = Point3f( q, p, V );

	    uchar b = (uchar)(std::max(0.f, std::min (res.x, 1.f)) * 255.f);
	    uchar g = (uchar)(std::max(0.f, std::min (res.y, 1.f)) * 255.f);
	    uchar r = (uchar)(std::max(0.f, std::min (res.z, 1.f)) * 255.f);

	    rgb.at<Point3_<uchar> >(y,x) = Point3_<uchar>(b, g, r);
	}
    }
}

static float getMaxDisparity( VideoCapture& cap )
{
    const int minDistance = 400; // mm
    float b = (float)cap.get( CAP_OPENNI_DEPTH_GENERATOR_BASELINE ); // mm
    float F = (float)cap.get( CAP_OPENNI_DEPTH_GENERATOR_FOCAL_LENGTH ); // pixels
    return b * F / minDistance;
}



static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
	selection.x = MIN(x, origin.x);
	selection.y = MIN(y, origin.y);
	selection.width = std::abs(x - origin.x);
	selection.height = std::abs(y - origin.y);

	selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
	case EVENT_LBUTTONDOWN: //choose the object
	    origin = Point(x,y);
	    selection = Rect(x,y,0,0);
	    selectObject = true;
	    break;
	case EVENT_LBUTTONUP: //track the object
	    selectObject = false;
	    if( selection.width > 0 && selection.height > 0 )
		trackObject = -1;
	    break;
    }
}

static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
	"You select a color objects such as your face and it tracks it.\n"
	"This reads from video camera (0 by default, or the camera number the user enters\n"
	"Usage: \n"
	"   ./camshiftdemo [camera number]\n";

    cout << "\n\nHot keys: \n"
	"\tESC - quit the program\n"
	"\tc - stop the tracking\n"
	"\tb - switch to/from backprojection view\n"
	"\th - show/hide object histogram\n"
	"\tp - pause video\n"
	"To initialize tracking, select the object with mouse\n";
}

const char* keys =
{
    "{@camera_number| 0 | camera number}"
};

int main( int argc, const char** argv )
{
    help();
    bool isFixedMaxDisp;
    isFixedMaxDisp = false;
    VideoCapture cap;
    Rect trackWindow;
    int hsize = 16; //number of bin
    float hranges[] = {0,180}; //range of pixel
    const float* phranges = hranges;
    CommandLineParser parser(argc, argv, keys);
    //int camNum = parser.get<int>(0);

    //cap.open(camNum);

    cap.open( CAP_OPENNI2 );
    if( !cap.isOpened() )
	cap.open( CAP_OPENNI );

    if( !cap.isOpened() )
    {
	help();
	cout << "***Could not initialize capturing...***\n";
	cout << "Current parameter's value: \n";
	parser.printMessage();
	return -1;
    }
#if 1
    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    //Window name, Mouse callback, The optional parameter passed to the callback

    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
#endif

    Mat depthMap;
    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;

#if 0 
    for(;;)
    {	
	if( !cap.grab() )
	{       
	    cout << "Can not grab images." << endl;
	    return -1;
	}
	else{
	    cap.retrieve( disparityMap, CAP_OPENNI_DISPARITY_MAP );
	    imshow( "dis ============", disparityMap);
	    Mat colorDisparityMap;
	   //colorizeDisparity( disparityMap, colorDisparityMap, isFixedMaxDisp ? getMaxDisparity(cap) : -1 );
	   // colorizeDisparity( disparityMap, colorDisparityMap, getMaxDisparity(cap)  );
	    colorDisparityMap.copyTo( frame, disparityMap != 0 );
	    imshow( "frame ==================", frame);
	}
	if( waitKey( 30 ) >= 0 )
	    break;

    }
    return 0;
#endif


    for(;;)
    {
	if( !paused )
	{
	    if( !cap.grab() )
	    {
		cout << "Can not grab images." << endl;
		return -1;
	    }
	    else{ 
		//adi
		cap.retrieve( depthMap, CAP_OPENNI_DEPTH_MAP );
		const float scaleFactor = 0.05f;
		Mat show; depthMap.convertTo( show, CV_8UC1, scaleFactor );

		////imshow( "depth map", show );
		//Joe add threshold
		//int threshold_type = THRESH_BINARY_INV;
		//int threshold_value = 80;
		//imshow( "&&&&&&&&&&&&&&&&&&", show );

		threshold(show, show, 30, 256, THRESH_TOZERO);
		threshold(show, show, 45, 256, THRESH_TOZERO_INV);
 		
		//imshow( "******************", show );

		adaptiveThreshold(show, show, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 3, -3);

		//imshow( "threshold depth map", show );

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
				//bounding_rect=boundingRect(contours[i]); 
				// Find the bounding rectangle for biggest contour
			}
		}		

	    Mat drawing = Mat::zeros(show.size(), CV_8UC3);
		
		//imshow("XXXXXXX", drawing);
		
		
		Mat drawing_line = Mat::zeros(show.size(), CV_8UC3);
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
		imshow("Contours", drawing);
		//imshow("XXXXXXXXXXContours", shapeRect);
		//imshow("Contours_Line", drawing_line);
		

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
		
	
		drawing.copyTo(frame);

		if( frame.empty())
		//if( image.empty())
		    break;
//		if( waitKey( 30 ) >= 0 )
//		break;
	    }   
	}
#if 1
    
//	cvtColor(frame, frame, COLOR_BGR2RGB); //adi
	frame.copyTo(image);
	//drawing.copyTo(image);

	if( !paused )
	{
	    
		cvtColor(image, hsv, COLOR_RGB2HSV); //change image (RGB) to HSV 
	    //cvtColor(image, hsv, COLOR_BGR2HSV); //change image (RGB) to HSV 


	    if( trackObject ) //trackObject = 0
	    {
		int _vmin = vmin, _vmax = vmax;

		inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
			Scalar(180, 256, MAX(_vmin, _vmax)), mask); 
		//H：0~180，S：smin~256，V：vmin~vmax

		int ch[] = {0, 0};
		hue.create(hsv.size(), hsv.depth());
		mixChannels(&hsv, 1, &hue, 1, ch, 1); 
		/*
		 * &hsv : input array or vector of matricesl
		 * 1    : number of matrices in src
		 * &hue : output array or vector of matrices
		 * 1    : number of matrices in &hue
		 * ch   : array of index pairs specifying which channels are copied 
		 * 1    : number of index pairs in ch
		 * 
		 */

		if( trackObject < 0 ) //If the object has no attribute extraction, choose the object
		{
		    Mat roi(hue, selection), maskroi(mask, selection); // Region of Interest (ROI)
		    //imshow("xxxxxxx", roi);
		    //imshow("yyyyyyy", maskroi);
		    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges); 
		    //Calculates a histogram of a set of arrays

		    normalize(hist, hist, 0, 255, NORM_MINMAX); 
		    //Normalizes the norm or value range of an array

		    trackWindow = selection;
		    trackObject = 1; //the object has attribute
		    histimg = Scalar::all(0); //all elements: 0
		    int binW = histimg.cols / hsize;
		    Mat buf(1, hsize, CV_8UC3);
		    for( int i = 0; i < hsize; i++ )
			buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
		    cvtColor(buf, buf, COLOR_HSV2RGB);
		    //cvtColor(buf, buf, COLOR_HSV2BGR);

		    for( int i = 0; i < hsize; i++ )
		    {
			int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
			rectangle( histimg, Point(i*binW,histimg.rows),
				Point((i+1)*binW,histimg.rows - val),
				Scalar(buf.at<Vec3b>(i)), -1, 8 );
		    }
		}

		calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
		backproj &= mask;
		RotatedRect trackBox = CamShift(backproj, trackWindow,
			TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));
		if( trackWindow.area() <= 1 )
		{
		    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
		    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
			    trackWindow.x + r, trackWindow.y + r) &
			Rect(0, 0, cols, rows);
		}

		if( backprojMode )
		    cvtColor( backproj, image, COLOR_GRAY2BGR );
		ellipse( image, trackBox, Scalar(0,0,255), 3, LINE_AA );
	    }
	}
	else if( trackObject < 0 )
	    paused = false;

	if( selectObject && selection.width > 0 && selection.height > 0 )
	{
	    Mat roi(image, selection);
	    bitwise_not(roi, roi);
	}

	imshow( "CamShift Demo", image );
	imshow( "Histogram", histimg );


	char c = (char)waitKey(10);
	if( c == 27 )
	    break;
	switch(c)
	{
	    case 'b':
		backprojMode = !backprojMode;
		break;
	    case 'c':
		trackObject = 0;
		histimg = Scalar::all(0);
		break;
	    case 'h':
		showHist = !showHist;
		if( !showHist )
		    destroyWindow( "Histogram" );
		else
		    namedWindow( "Histogram", 1 );
		break;
	    case 'p':
		paused = !paused;
		break;
	    default:
		;
	}
	}

    return 0;
#endif

}


