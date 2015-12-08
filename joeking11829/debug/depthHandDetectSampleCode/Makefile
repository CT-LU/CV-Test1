
all	:	
	g++ `pkg-config --cflags opencv` depth_image_generator.hpp depth_image_generator.cpp contours_operator.hpp contours_operator.cpp depth_hand_detection.cpp `pkg-config --libs opencv` -o build/app

clean :
	rm -rf build/*
