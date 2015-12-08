CXX ?= g++

CXXFLAGS += -c -Wall $(shell pkg-config --cflags opencv)
LDFLAGS += $(shell pkg-config --libs --static opencv)
LDFLAGS += "-L/usr/local/cuda-6.5/targets/x86_64-linux/lib/"

all: opencv_example

opencv_example: example.o; $(CXX) $< -o $@ $(LDFLAGS)

%.o: %.cpp; $(CXX) $< -o $@ $(CXXFLAGS)

clean: ; rm -f example.o opencv_example
