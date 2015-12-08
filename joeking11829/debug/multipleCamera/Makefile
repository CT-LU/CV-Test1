all:
	g++ depth_image_generator.cpp main.cpp  `pkg-config --cflags opencv` `pkg-config --libs opencv` -o ./build/multi-camera

clean:	
	rm -rf ./build/*
