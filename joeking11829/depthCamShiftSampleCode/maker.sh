
# Make 
g++ `pkg-config --cflags opencv` depth_image_generator.hpp depth_image_generator.cpp contours_operator.hpp contours_operator.cpp main_sample_01.cpp `pkg-config --libs opencv` -o build/app
