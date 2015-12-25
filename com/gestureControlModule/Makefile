OPENCVFLAG = `pkg-config --cflags opencv`
OPENCVLIBS = `pkg-config --libs opencv`

CAMERA_DIR = ../camera/

INC_PATH = -I$(CAMERA_DIR)
LIBS = $(OPENCVFLAG)
FLAG = $(OPENCVLIBS)

all:gesture_app

$(CAMERA_DIR)/XtionCam.o:$(CAMERA_DIR)
	$(MAKE) -C $(CAMERA_DIR)

gesture_app:contours_operator.cpp gesture_control.cpp test.cpp $(CAMERA_DIR)/XtionCam.o
	g++ $? $(INC_PATH) $(LIBS) $(FLAG) -o $@  
	
clean:
	rm -rf *.o gesture_app

