
all	:	
	g++ `pkg-config --cflags opencv` depth_image_generator.hpp depth_image_generator.cpp contours_operator.hpp contours_operator.cpp main.cpp `pkg-config --libs opencv` -o build/gesture_app

clean :
	rm -rf build/*
