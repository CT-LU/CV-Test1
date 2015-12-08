
all	:	
	g++ `pkg-config --cflags opencv` sobel.hpp sobel.cpp findcontours.hpp findcontours.cpp main.cpp `pkg-config --libs opencv` -o build/output

	
clean :
	rm -rf build/output


