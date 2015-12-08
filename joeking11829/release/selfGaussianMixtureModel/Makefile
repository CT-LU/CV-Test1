all:
	g++ main.cpp -L/usr/lib -linput `pkg-config --cflags opencv` `pkg-config --libs opencv` -o ./build/gmm

clean:	
	rm -rf ./build/*
