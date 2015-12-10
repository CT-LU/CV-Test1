all:
	g++ main.cpp -o c_python -I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7 -fno-strict-aliasing -D_FORTIFY_SOURCE=2 -g -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -L/usr/lib/python2.7/config-x86_64-linux-gnu -L/usr/lib -lpthread -ldl -lutil -lm -lpython2.7 -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions `pkg-config --cflags opencv` `pkg-config --libs opencv`
	
clean:
	rm *.o
	rm a.out

#use following instructions to get the argumets above
#python-config --cflags
#python-config --ldflags 
