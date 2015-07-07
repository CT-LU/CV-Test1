#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace cv;
using namespace std;

static void help()
{
	cout << "\nThis program demonstrates usage of depth sensors (Kinect, XtionPRO,...).\n"
		"The user gets some of the supported output images.\n"
		"\nAll supported output map types:\n"
		"1.) Data given from depth generator\n"
		"   CAP_OPENNI_DEPTH_MAP            - depth values in mm (CV_16UC1)\n"
		"   CAP_OPENNI_POINT_CLOUD_MAP      - XYZ in meters (CV_32FC3)\n"
		"   CAP_OPENNI_DISPARITY_MAP        - disparity in pixels (CV_8UC1)\n"
		"   CAP_OPENNI_DISPARITY_MAP_32F    - disparity in pixels (CV_32FC1)\n"
		"   CAP_OPENNI_VALID_DEPTH_MASK     - mask of valid pixels (not ocluded, not shaded etc.) (CV_8UC1)\n"
		"2.) Data given from RGB image generator\n"
		"   CAP_OPENNI_BGR_IMAGE            - color image (CV_8UC3)\n"
		"   CAP_OPENNI_GRAY_IMAGE           - gray image (CV_8UC1)\n"
		<< endl;
}

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

static float getMaxDisparity( VideoCapture& capture )
{
	const int minDistance = 400; // mm
	float b = (float)capture.get( CAP_OPENNI_DEPTH_GENERATOR_BASELINE ); // mm
	float F = (float)capture.get( CAP_OPENNI_DEPTH_GENERATOR_FOCAL_LENGTH ); // pixels
	return b * F / minDistance;
}

static void printCommandLineParams()
{
	cout << "-cd       Colorized disparity? (0 or 1; 1 by default) Ignored if disparity map is not selected to show." << endl;
	cout << "-fmd      Fixed max disparity? (0 or 1; 0 by default) Ignored if disparity map is not colorized (-cd 0)." << endl;
	cout << "-mode     image mode: resolution and fps, supported three values:  0 - CAP_OPENNI_VGA_30HZ, 1 - CAP_OPENNI_SXGA_15HZ," << endl;
	cout << "          2 - CAP_OPENNI_SXGA_30HZ (0 by default). Ignored if rgb image or gray image are not selected to show." << endl;
	cout << "-m        Mask to set which output images are need. It is a string of size 5. Each element of this is '0' or '1' and" << endl;
	cout << "          determine: is depth map, disparity map, valid pixels mask, rgb image, gray image need or not (correspondently)?" << endl ;
	cout << "          By default -m 01010 i.e. disparity map and rgb image will be shown." << endl ;
	cout << "-r        Filename of .oni video file. The data will grabbed from it." << endl ;
}

static void parseCommandLine( int argc, char* argv[], bool& isColorizeDisp, bool& isFixedMaxDisp, int& imageMode, bool retrievedImageFlags[],
		string& filename, bool& isFileReading )
{
	// set defaut values
	isColorizeDisp = true;
	isFixedMaxDisp = false;
	imageMode = 0;

	retrievedImageFlags[0] = false;
	retrievedImageFlags[1] = true;
	retrievedImageFlags[2] = false;
	retrievedImageFlags[3] = true;
	retrievedImageFlags[4] = false;

	filename.clear();
	isFileReading = false;

	if( argc == 1 )
	{
		help();
	}
	else
	{
		for( int i = 1; i < argc; i++ )
		{
			if( !strcmp( argv[i], "--help" ) || !strcmp( argv[i], "-h" ) )
			{
				printCommandLineParams();
				exit(0);
			}
			else if( !strcmp( argv[i], "-cd" ) )
			{
				isColorizeDisp = atoi(argv[++i]) == 0 ? false : true;
			}
			else if( !strcmp( argv[i], "-fmd" ) )
			{
				isFixedMaxDisp = atoi(argv[++i]) == 0 ? false : true;
			}
			else if( !strcmp( argv[i], "-mode" ) )
			{
				imageMode = atoi(argv[++i]);
			}
			else if( !strcmp( argv[i], "-m" ) )
			{
				string mask( argv[++i] );
				if( mask.size() != 5)
					CV_Error( Error::StsBadArg, "Incorrect length of -m argument string" );
				int val = atoi(mask.c_str());

				int l = 100000, r = 10000, sum = 0;
				for( int j = 0; j < 5; j++ )
				{
					retrievedImageFlags[j] = ((val % l) / r ) == 0 ? false : true;
					l /= 10; r /= 10;
					if( retrievedImageFlags[j] ) sum++;
				}

				if( sum == 0 )
				{
					cout << "No one output image is selected." << endl;
					exit(0);
				}
			}
			else if( !strcmp( argv[i], "-r" ) )
			{
				filename = argv[++i];
				isFileReading = true;
			}
			else
			{
				cout << "Unsupported command line argument: " << argv[i] << "." << endl;
				exit(-1);
			}
		}
	}
}

/*
 * To work with Kinect or XtionPRO the user must install OpenNI library and PrimeSensorModule for OpenNI and
 * configure OpenCV with WITH_OPENNI flag is ON (using CMake).
 */
int main( int argc, char* argv[] )
{
	bool isColorizeDisp, isFixedMaxDisp;
	int imageMode;
	bool retrievedImageFlags[5];
	string filename;
	bool isVideoReading;
	parseCommandLine( argc, argv, isColorizeDisp, isFixedMaxDisp, imageMode, retrievedImageFlags, filename, isVideoReading );

	cout << "Device opening ..." << endl;
	VideoCapture capture;
	if( isVideoReading )
		capture.open( filename );
	else
	{
		capture.open( CAP_OPENNI2 );
		if( !capture.isOpened() )
			capture.open( CAP_OPENNI );
	}

	cout << "done." << endl;

	if( !capture.isOpened() )
	{
		cout << "Can not open a capture object." << endl;
		return -1;
	}

	if( !isVideoReading )
	{
		bool modeRes=false;
		switch ( imageMode )
		{
			case 0:
				modeRes = capture.set( CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CAP_OPENNI_VGA_30HZ );
				break;
			case 1:
				modeRes = capture.set( CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CAP_OPENNI_SXGA_15HZ );
				break;
			case 2:
				modeRes = capture.set( CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CAP_OPENNI_SXGA_30HZ );
				break;
				//The following modes are only supported by the Xtion Pro Live
			case 3:
				modeRes = capture.set( CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CAP_OPENNI_QVGA_30HZ );
				break;
			case 4:
				modeRes = capture.set( CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CAP_OPENNI_QVGA_60HZ );
				break;
			default:
				CV_Error( Error::StsBadArg, "Unsupported image mode property.\n");
		}
		if (!modeRes)
			cout << "\nThis image mode is not supported by the device, the default value (CV_CAP_OPENNI_SXGA_15HZ) will be used.\n" << endl;
	}

	// Print some avalible device settings.
	cout << "\nDepth generator output mode:" << endl <<
		"FRAME_WIDTH      " << capture.get( CAP_PROP_FRAME_WIDTH ) << endl <<
		"FRAME_HEIGHT     " << capture.get( CAP_PROP_FRAME_HEIGHT ) << endl <<
		"FRAME_MAX_DEPTH  " << capture.get( CAP_PROP_OPENNI_FRAME_MAX_DEPTH ) << " mm" << endl <<
		"FPS              " << capture.get( CAP_PROP_FPS ) << endl <<
		"REGISTRATION     " << capture.get( CAP_PROP_OPENNI_REGISTRATION ) << endl;
	if( capture.get( CAP_OPENNI_IMAGE_GENERATOR_PRESENT ) )
	{
		cout <<
			"\nImage generator output mode:" << endl <<
			"FRAME_WIDTH   " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CAP_PROP_FRAME_WIDTH ) << endl <<
			"FRAME_HEIGHT  " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CAP_PROP_FRAME_HEIGHT ) << endl <<
			"FPS           " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CAP_PROP_FPS ) << endl;
	}
	else
	{
		cout << "\nDevice doesn't contain image generator." << endl;
		if (!retrievedImageFlags[0] && !retrievedImageFlags[1] && !retrievedImageFlags[2])
			return 0;
	}

	for(;;)
	{
		Mat depthMap;
		Mat validDepthMap;
		Mat disparityMap;
		Mat bgrImage;
		Mat grayImage;

		if( !capture.grab() )
		{
			cout << "Can not grab images." << endl;
			return -1;
		}
		else
		{
			if( retrievedImageFlags[0] && capture.retrieve( depthMap, CAP_OPENNI_DEPTH_MAP ) )
			{
				const float scaleFactor = 0.05f;
				Mat show; depthMap.convertTo( show, CV_8UC1, scaleFactor );

				imshow( "depth map", show );

				//Joe add threshold
				//int threshold_type = THRESH_BINARY_INV;
				//int threshold_value = 80;

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
				
				/*
				 * Mark for using Histogram
				 *
				
				//Calculate Histogram
				int histSize = 256;
				float range[] = { 0, 255 };
				const float* histRange = { range };

				bool uniform = true; 
				bool accumulate = false;

				Mat hist;
				//Compute the histograms:
				calcHist( &drawing, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );


				// Draw the histograms for B, G and R
				int hist_w = 512; int hist_h = 400;
				int bin_w = cvRound( (double) hist_w/histSize );

				Mat histImage( hist_h, hist_w, CV_8UC1, Scalar( 0,0,0) );
				/// Draw for each channel
				for( int i = 1; i < histSize; i++ )
				{
					line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
							Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
							Scalar( 255, 0, 0), 2, 8, 0  );
				}

				/// Display
				imshow("calcHist Demo", histImage );

				*
				*/



				//Joe End

			}

			if( retrievedImageFlags[1] && capture.retrieve( disparityMap, CAP_OPENNI_DISPARITY_MAP ) )
			{
				if( isColorizeDisp )
				{
					Mat colorDisparityMap;
					colorizeDisparity( disparityMap, colorDisparityMap, isFixedMaxDisp ? getMaxDisparity(capture) : -1 );
					Mat validColorDisparityMap;
					colorDisparityMap.copyTo( validColorDisparityMap, disparityMap != 0 );
					imshow( "colorized disparity map", validColorDisparityMap );
				}
				else
				{
					imshow( "original disparity map", disparityMap );
				}
			}

			if( retrievedImageFlags[2] && capture.retrieve( validDepthMap, CAP_OPENNI_VALID_DEPTH_MASK ) )
				imshow( "valid depth mask", validDepthMap );

			if( retrievedImageFlags[3] && capture.retrieve( bgrImage, CAP_OPENNI_BGR_IMAGE ) )
				imshow( "rgb image", bgrImage );

			if( retrievedImageFlags[4] && capture.retrieve( grayImage, CAP_OPENNI_GRAY_IMAGE ) )
				imshow( "gray image", grayImage );
		}

		if( waitKey( 30 ) >= 0 )
			break;
	}

	return 0;
}
