SUBDIRS = invokeDetectModule MaskWithCudaGmm 

CVINCLUDE = `pkg-config --cflags opencv` 
CVLIB = `pkg-config --libs opencv`

OBJS = ./invokeDetectModule/hand_detect.o ./MaskWithCudaGmm/MaskWithCudaGmm.o

LIBS = $(CVLIB) -L./MaskWithCudaGmm/ -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib 
INC = $(CVINCLUDE) -I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7 

CFLAGS = -fno-strict-aliasing -D_FORTIFY_SOURCE=2 -g -fstack-protector --param=ssp-buffer-size=4 \
	 -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes \
       	 -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions

LDLIBS = -lgmmWithCuda -lpthread -ldl -lutil -lm -lpython2.7

all: handGesture

$(OBJS): $(SUBDIRS)
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir; done

handGesture: $(OBJS) main.cpp 
	g++ $? $(LIBS) $(LDLIBS) $(INC) $(CFLAGS) -o $@ 

clean:                             
	for dir in $(SUBDIRS); do $(MAKE) clean -C $$dir; done
	rm -rf *.o handGesture

