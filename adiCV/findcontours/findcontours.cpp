#include "opencv2/videoio/videoio.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "findcontours.hpp"

#include <iostream>

using namespace cv;
using namespace std;



void function3_1(int &x, int &y, int &x1, int &y1){
	x1 = x - 1;
	y1 = y - 1;
}

void function3_2(int x, int y, int &x1, int &y1, int &x2, int &y2, int &x3, int &y3, int **examined, int image_height, int image_width){
	x2 = x1;
	y2 = y1;
	x3 = x;
	y3 = y;
	examined[y-1][x-1] = 1;
}

void function3_4(int &x3, int &y3, int &comNBD, int **pixelNBD, int **examined, vector<Point> &vector_point, int image_height, int image_width){
	if(pixelNBD[y3][x3+1] == 0 && examined[y3][x3+1] == 1){
		pixelNBD[y3][x3] = -comNBD;
		vector_point.push_back(Point(x3,y3));
	}else if(pixelNBD[y3][x3+1] != 0 && examined[y3][x3+1] == 1 && pixelNBD[y3][x3] == 1){
		pixelNBD[y3][x3] = comNBD;
		vector_point.push_back(Point(x3,y3));
	}else
	{
		pixelNBD[y3][x3] = comNBD;
		vector_point.push_back(Point(x3,y3));
	}
}

void function3_5(int &x2, int &y2, int &x3, int &y3, int &x4, int &y4){						
	x2 = x3;
	y2 = y3;
	x3 = x4;
	y3 = y4;
}



int findNewCenter(int &x3, int &y3, int &x4, int &y4, int **pixelNBD, int **examined, int x_shift, int y_shift) {
	if(pixelNBD[y3 + y_shift][x3 + x_shift] == 0) return 0; 
	x4 = x3 + x_shift;
	y4 = y3 + y_shift;
	examined[y3 + y_shift][x3 + x_shift] = 1;						
	return 1;
}

void find_point(int &x2, int &y2, int &x3, int &y3, int &x4, int &y4, int **pixelNBD, int **examined){
	if((x2 > x3) && (y2 == y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0);
	}
	else if((x2 > x3) && (y2 < y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1);
	}
	else if((x2 == x3) && (y2 < y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1);
	}
	else if((x2 < x3) && (y2 < y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1);
	}
	else if((x2 < x3) && (y2 == y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0);
	}
	else if((x2 < x3) && (y2 > y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1);
	}
	else if((x2 == x3) && (y2 > y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1);
	}
	else if((x2 > x3) && (y2 > y3)){
		if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, 0))
			if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, -1))
				if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, -1))
					if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, -1))
						if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, 0))
							if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, -1, +1))
								if(!findNewCenter(x3, y3, x4, y4, pixelNBD, examined, 0, +1))
									findNewCenter(x3, y3, x4, y4, pixelNBD, examined, +1, +1);
	}		
}

void delete2dArray(int **Array, int image_height, int image_width){
	for(int i = 0; i < image_height; i++) {
		delete [] Array[i];
	}
	delete [] Array;
}


