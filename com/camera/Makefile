CVINCLUDE = `pkg-config --cflags opencv` 
CVLIB = `pkg-config --libs opencv`

LIBS = $(CVLIB)
INC = $(CVINCLUDE)

all: xtion_test 

XtionCam.o:XtionCam.cpp XtionCam.hpp ICamera.hpp
	g++ $< $(LIBS) $(INC) -c

xtion_test: test.cpp XtionCam.o 
	g++ $? $(LIBS) $(INC) -o $@ 

clean:                             
	rm -rf *.o xtion_test

