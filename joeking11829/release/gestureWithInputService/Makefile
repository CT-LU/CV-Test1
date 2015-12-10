all:
	g++ depth_image_generator.cpp contours_operator.cpp gesture_control.cpp main.cpp `pkg-config --cflags opencv` `pkg-config --libs opencv` -o ./build/gesture_app

clean:	
	rm -rf ./build/*
