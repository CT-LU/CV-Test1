all:
	g++ depth_image_generator.cpp contours_operator.cpp main.cpp -L/usr/lib -linput `pkg-config --cflags opencv` `pkg-config --libs opencv` -o ./build/gesture_app

clean:	
	rm -rf ./build/*
