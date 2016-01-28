FROM joe/cuda-x2go:stable

MAINTAINER JOE YOUNG <joeking11829@gmail.com>

# Install OpenCV dependency lib
RUN sudo apt-get update
RUN sudo apt-get install -y python-dev python-pip python-numpy build-essential cmake pkg-config libatlas-base-dev gfortran \
                            libjasper-dev libgtk2.0-dev libavcodec-dev libavformat-dev \
                            libswscale-dev libjpeg-dev libpng-dev libtiff-dev libv4l-dev v4l-utils

# Install Python Module
RUN sudo pip install numpy matplotlib

#Set libcuda.so
RUN rm -rf /usr/lib/x86_64-linux-gnu/libcuda.so
RUN ln -s /usr/local/cuda/lib64/stubs/libcuda.so /usr/lib/x86_64-linux-gnu/libcuda.so

# Change User
USER gesture
WORKDIR /home/gesture

# Download PrimeSense Binary
RUN mkdir xtion_pro
WORKDIR /home/gesture/xtion_pro
RUN wget https://github.com/joeking11829/CameraPSB/blob/master/Bin/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23.tar.bz2?raw=true -O OpenNI-Bin-Dev-Linux-x64-v1.5.2.23.tar.bz2
RUN wget https://github.com/joeking11829/CameraPSB/blob/master/Bin/Sensor-Bin-Linux-x64-v5.1.0.41.tar.bz2?raw=true -O Sensor-Bin-Linux-x64-v5.1.0.41.tar.bz2
RUN tar jxvf OpenNI-Bin-Dev-Linux-x64-v1.5.2.23.tar.bz2
RUN tar jxvf Sensor-Bin-Linux-x64-v5.1.0.41.tar.bz2

# Install PrimeSense Lib
USER root
RUN cd OpenNI-Bin-Dev-Linux-x64-v1.5.2.23 && sudo ./install.sh
RUN cd Sensor-Bin-Linux-x64-v5.1.0.41 && sudo ./install.sh
USER gesture
WORKDIR /home/gesture

# Download OpenCV
RUN wget https://github.com/Itseez/opencv/archive/3.0.0.zip -O opencv-3.0.0.zip && \
    unzip -q opencv-3.0.0.zip

# Build OpenCV
RUN mkdir opencv-3.0.0/release
WORKDIR /home/gesture/opencv-3.0.0/release
RUN cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D WITH_V4L=ON \
          -D WITH_IPP=OFF \
          -D WITH_OPENNI=ON \
          ..

#          -D INSTALL_C_EXAMPLES=ON \
#          -D INSTALL_PYTHON_EXAMPLES=ON \
#          -D BUILD_EXAMPLES=ON \

RUN make -j8

# Install OpenCV Lib
USER root
RUN sudo make install
RUN sudo ldconfig
RUN echo "usr/local/liib" >> /etc/ld.so.conf.d/opencv.conf && sudo ldconfig -v
RUN export LD_LIBRARY_PATH=/usr/local/lib/:$LD_LIBRARY_PATH && sudo ldconfig
RUN export LD_LIBRARY_PATH=/home/gesture/opencv-3.0.0/release/:$LD_LIBRARY_PATH && sudo ldconfig
RUN echo "export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig" >> /etc/bash.bashrc
# Install OpenCV-Python2.7 cv2.so
RUN sudo cp lib/cv2.so /usr/lib/python2.7/dist-packages/.

# ReSet libcuda.so
RUN rm -rf /usr/lib/x86_64-linux-gnu/libcuda.so
RUN ln -s /usr/local/nvidia/lib64/libcuda.so.1 /usr/lib/x86_64-linux-gnu/libcuda.so

# Remove no-use data
RUN sudo rm -rf /var/lib/apt/lists/*

EXPOSE 22

# Start sshd
CMD /usr/sbin/sshd && /bin/bash
