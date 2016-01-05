OPENCVFLAG = `pkg-config --cflags opencv`
OPENCVLIBS = `pkg-config --libs opencv`

CAMERA_DIR = ../camera/

INC_PATH = -I$(CAMERA_DIR)
LIBS = $(OPENCVFLAG)
FLAG = $(OPENCVLIBS)

all: gesture_app

$(CAMERA_DIR)/XtionCam.o:$(CAMERA_DIR)
	$(MAKE) -C $(CAMERA_DIR)

contours_operator.o: contours_operator.cpp
	g++ $< $(LIBS) $(FLAG) -c

gesture_control.o: gesture_control.cpp
	g++ $< $(LIBS) $(FLAG) -c

gesture_app: test.cpp contours_operator.o gesture_control.o $(CAMERA_DIR)/XtionCam.o
	g++ $? $(INC_PATH) $(LIBS) $(FLAG) -o $@  
	
clean:
	rm -rf *.o gesture_app

