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
int vmin = 10, vmax = 256, smin = 30;


int main( int argc, constf char** argv )
{
	Rect trackWindow;
	int hsize = 16;
	float hranges[] = {0,180};
	const float* phranges = hranges;

	namedWindow( "Histogram", 0 );
	namedWindow( "CamShift Demo", 0 );
	setMouseCallback( "CamShift Demo", onMouse, 0 );
	createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
	createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
	createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

	Mat frame, hsv, hue, mask;
	Mat hist, histimg = Mat::zeros(200, 320, CV_8UC3);
	Mat backproj;

	for(;;)
	{

		//Get depth image
		//
		getDepthCamera(&frame);

		if( image.empty() )
		{
			break;
		}

		frame.copyTo(image);

		//Color Space transform -> from BGR to HSV
		cvtColor(image, hsv, COLOR_BGR2HSV);

		if( trackObject )
		{
			int _vmin = vmin, _vmax = vmax;

			inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
					Scalar(180, 256, MAX(_vmin, _vmax)), mask);
			int ch[] = {0, 0};
			hue.create(hsv.size(), hsv.depth());
			mixChannels(&hsv, 1, &hue, 1, ch, 1);

			if( trackObject < 0 )
			{
				Mat roi(hue, selection), maskroi(mask, selection);
				calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
				normalize(hist, hist, 0, 255, NORM_MINMAX);

				trackWindow = selection;
				trackObject = 1;

				histimg = Scalar::all(0);
				int binW = histimg.cols / hsize;
				Mat buf(1, hsize, CV_8UC3);

				for( int i = 0; i < hsize; i++ )
				{
					buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
				}
				cvtColor(buf, buf, COLOR_HSV2BGR);

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
			{
				cvtColor( backproj, image, COLOR_GRAY2BGR );
			}

			//Draw focus shape
			ellipse( image, trackBox, Scalar(0,0,255), 3, LINE_AA );
		}


	}

	return 0;
}
