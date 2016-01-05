CVINCLUDE = `pkg-config --cflags opencv`
CVLIB = `pkg-config --libs opencv`

CAMERA_DIR = ../camera/
HAND_DETECT_DIR = ../invokeDetectModule/
GESTURE_CONTROL_DIR = ../gestureControlModule/

INC = $(CVINCLUDE)
INC += -I$(CAMERA_DIR)
INC += -I$(HAND_DETECT_DIR)
INC += -I$(GESTURE_CONTROL_DIR)
INC += -I/usr/include/python2.7
INC += -I/usr/include/x86_64-linux-gnu/python2.7

LIBS = $(CVLIB) 
LIBS += -L/usr/lib/python2.7/config-x86_64-linux-gnu
LIBS += -L/usr/lib

CFLAGS = -fno-strict-aliasing -D_FORTIFY_SOURCE=2 -g -fstack-protector --param=ssp-buffer-size=4 \
	 -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes \
       	 -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions

LDLIBS = -lpthread -ldl -lutil -lm -lpython2.7

all: cube_generator

$(CAMERA_DIR)/XtionCam.o:$(CAMERA_DIR)
	$(MAKE) -C $(CAMERA_DIR)

$(HAND_DETECT_DIR)/hand_detect.o $(HAND_DETECT_DIR)/CPythonUtil.o: $(HAND_DETECT_DIR)
	$(MAKE) -C $(HAND_DETECT_DIR)

$(GESTURE_CONTROL_DIR)/contours_operator.o: $(GESTURE_CONTROL_DIR)
	$(MAKE) -C $(GESTURE_CONTROL_DIR)

CubeGenerator.o: CubeGenerator.cpp
	g++ $< $(LIBS) $(INC) $(CFLAG) -c

cube_generator: test.cpp CubeGenerator.o $(GESTURE_CONTROL_DIR)/contours_operator.o $(HAND_DETECT_DIR)/hand_detect.o $(HAND_DETECT_DIR)/CPythonUtil.o $(CAMERA_DIR)/XtionCam.o
	g++ $? $(LIBS) $(LDLIBS) $(INC) $(CFLAG) -o $@  
	
clean:
	rm -rf *.o cube_generator

