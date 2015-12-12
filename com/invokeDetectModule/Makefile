CVINCLUDE = `pkg-config --cflags opencv` 
CVLIB = `pkg-config --libs opencv`

INC = -I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7 $(CVINCLUDE)
CFLAGS = -fno-strict-aliasing -D_FORTIFY_SOURCE=2 -g -fstack-protector --param=ssp-buffer-size=4 \
	-Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes \
	-Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions

LIBS = -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpthread -ldl -lutil -lm -lpython2.7 \
       $(CVLIB)

all: c_invoke_py

hand_detect.o: hand_detect.cpp
	g++ $? $(INC) $(CFLAGS) $(LIBS) -c

c_invoke_py: test.cpp hand_detect.o
	g++ $? $(INC) $(CFLAGS) $(LIBS) -o $@
	
clean:
	rm -rf *.o c_invoke_py

#use following instructions to get the argumets above
#python-config --cflags
#python-config --ldflags 
