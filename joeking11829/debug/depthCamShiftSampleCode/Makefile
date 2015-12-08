#Makefile

all :
	g++ `pkg-config --cflags opencv` depth_image_generator.hpp depth_image_generator.cpp contours_operator.hpp contours_operator.cpp main_sample_01.cpp `pkg-config --libs opencv` -o build/app

#all :
#	g++ `pkg-config --cflags opencv` depth_image_generator.hpp contours_operator.hpp main_sample_01.cpp `pkg-config --libs opencv` -o build/app

clean :
	rm -rf build/*


