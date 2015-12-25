CC = nvcc  		# compiler for compiling cuda
HOST_CC = g++		# compiler for compiling c or cpp 

CXXFLAGS = -O3 `pkg-config --cflags opencv` `pkg-config --libs opencv`		# parameters for cpp
CFLAGS = -O3  -Xcompiler -fPIC --shared 					# parameters for cuda 
INC = -I./									# include path
LIB = -L/usr/lib								# library path for cuda
CXXLIB = -L./ -L/usr/lib -lgmmWithCuda						# library path for cpp
       
all:libgmmWithCuda.so mask

libgmmWithCuda.so: gmmWithCuda.o
	${CC} $< ${CFLAGS} ${INC} ${LIB} -o $@

gmmWithCuda.o: gmmWithCuda.cu gmmWithCuda.h
	${CC} $< ${CFLAGS} ${INC} ${LIB} -c          

MaskWithCudaGmm.o:MaskWithCudaGmm.cpp MaskWithCudaGmm.hpp
	${HOST_CC} $< ${CXXFLAGS} ${INC} ${CXXLIB} -c          

mask: test.cpp MaskWithCudaGmm.o
	${HOST_CC} $? ${CXXFLAGS} ${INC} ${CXXLIB} -o $@

install:libgmmWithCuda.so
	cp $? /usr/local/lib
clean:                             
	rm -rf *.o libgmm*.* mask
