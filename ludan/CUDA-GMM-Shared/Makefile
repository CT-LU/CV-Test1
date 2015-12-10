CC = nvcc 
HOST_CC = g++ 

CXXFLAGS = -O3 `pkg-config --cflags opencv` `pkg-config --libs opencv`
CFLAGS = -O3  -Xcompiler -fPIC --shared `pkg-config --cflags opencv` `pkg-config --libs opencv`
INC = -I./ 
LIB = -L/usr/lib
CXXLIB = -L./ -L/usr/lib -lgmmWithCuda
       
all:libgmmWithCuda.so gmmWithCuda

libgmmWithCuda.so: gmmWithCuda.o
	${CC} gmmWithCuda.o ${CFLAGS} ${INC} ${LIB} -o $@

%.o: %.cu gmmWithCuda.h
	${CC} $< ${CFLAGS} ${INC} ${LIB} -c          

gmmWithCuda: main.cpp libgmmWithCuda.so
	${HOST_CC} $< ${CXXFLAGS} ${INC} ${CXXLIB} -o $@

.PHONY: clean
clean:                             
	rm -rf *.o libgmm*.*
	rm gmmWithCuda
