all:
	g++ main.cpp v4l2_grabber.c -L/usr/lib -lv4l2 `pkg-config --cflags opencv` `pkg-config --libs opencv` -o gmm

clean:	
	rm -rf ./build/*
