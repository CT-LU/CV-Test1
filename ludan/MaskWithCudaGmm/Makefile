CC = nvcc 
HOST_CC = g++ 

CXXFLAGS = -O3 `pkg-config --cflags opencv` `pkg-config --libs opencv`
CFLAGS = -O3  -Xcompiler -fPIC --shared
INC = -I./ 
LIB = -L/usr/lib
CXXLIB = -L./ -L/usr/lib -lgmmWithCuda
       
all:libgmmWithCuda.so mask 

libgmmWithCuda.so: gmmWithCuda.o
	${CC} gmmWithCuda.o ${CFLAGS} ${INC} ${LIB} -o $@

gmmWithCuda.o: gmmWithCuda.cu gmmWithCuda.h
	${CC} $< ${CFLAGS} ${INC} ${LIB} -c          

MaskWithCudaGmm.o:MaskWithCudaGmm.cpp MaskWithCudaGmm.hpp
	${HOST_CC} $< ${CXXFLAGS} ${INC} ${CXXLIB} -c          

mask: main.cpp libgmmWithCuda.so MaskWithCudaGmm.o
	${HOST_CC} $< MaskWithCudaGmm.o ${CXXFLAGS} ${INC} ${CXXLIB} -o $@

.PHONY: clean
clean:                             
	rm -rf *.o libgmm*.*
	rm mask
