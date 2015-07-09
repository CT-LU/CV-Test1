#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;


int startCamShiftTracking(Mat* image, Mat* hist, Mat* backproj, const float* phranges, Rect* trackWindow)
{
	
	calcBackProject(image, 1, 0, *hist, *backproj, &phranges);
	
	backproj &= mask;
	RotatedRect trackBox = CamShift(*backproj, *trackWindow,
			TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));

	if( trackWindow->area() <= 1 )
	{

		cout << "backproj->cols: " << backproj->cols << "backproj->rows: " << backproj->rows << endl;

		int cols = backproj->cols;
		int  rows = backproj->rows;
		int  r = (MIN(cols, rows) + 5)/6;
		
		*trackWindow = Rect(trackWindow->x - r, trackWindow->y - r,
				trackWindow->x + r, trackWindow->y + r) &
			Rect(0, 0, cols, rows);
	}

	//Draw focus shape
	ellipse( *image, trackBox, Scalar(0,0,255), 3, LINE_AA );

	return 0;
}

