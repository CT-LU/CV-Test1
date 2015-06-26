#Makefile practice

CXX=g++
CC=gcc

SRC= hand_detection.cpp
OBJ= hand_detection.o

CFLAGS=-I/usr/local/include/opencv -I/usr/local/include

LIBS=-L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core -lopencv_hal

OUT_DIR=build
OUT_OBJ=$(addprefix $(OUT_DIR)/, $(OBJ))


all : $(OBJ) $(OUT_DIR)
	$(CXX) $(OUT_OBJ) $(LIBS) -o $(OUT_DIR)/app

$(OBJ) : $(SRC) $(OUT_DIR)
	$(CXX) $(FLAGS) -c $(SRC) -o $(OUT_DIR)/$(OBJ)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean :
	rm -rf $(OUT_DIR)/*


#g++ `pkg-config --cflags opencv` MouseManipulator.h MouseManipulator.cpp main.cpp `pkg-config --libs opencv` -o app
