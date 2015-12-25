all:
	g++ depth_image_generator.cpp contours_operator.cpp gesture_control.cpp test.cpp -o gesture_app `pkg-config --cflags opencv` `pkg-config --libs opencv`
	
clean:
	rm -rf *.o gesture_app

#use following instructions to get the argumets above
#python-config --cflags
#python-config --ldflags 
