all:
	g++ depth_image_generator.cpp contours_operator.cpp gesture_control.cpp hand_detect.cpp MaskWithCudaGmm.cpp main.cpp -o gesture_app -I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7 -fno-strict-aliasing -D_FORTIFY_SOURCE=2 -g -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lgmmWithCuda -lpthread -ldl -lutil -lm -lpython2.7 -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions `pkg-config --cflags opencv` `pkg-config --libs opencv`
	
clean:
	rm -rf *.o gesture_app

#use following instructions to get the argumets above
#python-config --cflags
#python-config --ldflags 
