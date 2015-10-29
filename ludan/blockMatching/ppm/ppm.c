#include "ppm.h"

void changeColorPPM(PPMImage *img)
{
	int i;
	if(img){

		for( i = 0; i < img->x * img->y; i++){
			unsigned char* pixel = (unsigned char*)&img->data[i];
			pixel[0] = RGB_COMPONENT_COLOR - pixel[0];
			pixel[1] = RGB_COMPONENT_COLOR - pixel[1];
			pixel[2] = RGB_COMPONENT_COLOR - pixel[2];
		}
	}
}

int main(){
	PPMImage *image;
	image = readPPM("stereo.im0.450x375.ppm");
	changeColorPPM(image);
	writePPM("can_bottom.ppm",image);

	free(image->data);
	free(image);
}

