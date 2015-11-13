all:
	g++ depth_image_generator.cpp main.cpp -L/usr/lib -linput `pkg-config --cflags opencv` `pkg-config --libs opencv` -o ./build/multi-camera

clean:	
	rm -rf ./build/*
