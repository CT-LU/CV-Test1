FROM nvidia/cuda:7.5-cudnn3-devel-ubuntu14.04

MAINTAINER JOE YOUNG <joeking11829@gmail.com>

# Create USER
RUN useradd -d /home/gesture -m -s /bin/bash gesture
RUN adduser gesture sudo
RUN usermod -a -G video gesture

# Set NVIDIA CONFIG
ENV CUDA_HOME /usr/local/cuda
ENV PATH ${CUDA_HOME}/bin:/usr/local/sbin:/usr/sbin:/sbin:${PATH}
ENV LD_LIBRARY_PATH ${CUDA_HOME}/lib:${CUDA_HOME}/lib64:/usr/lib:/usr/local/lib:${LD_LIBRARY_PATH}

# Set libcuda.so
RUN rm -rf /usr/lib/x86_64-linux-gnu/libcuda.so
RUN ln -s /usr/local/nvidia/lib64/libcuda.so.1 /usr/lib/x86_64-linux-gnu/libcuda.so

#USER gesture
WORKDIR /home/gesture

# Install X2GO
RUN sudo apt-get update && sudo apt-get install -y software-properties-common python-software-properties openssh-server
RUN sudo add-apt-repository ppa:x2go/stable -y && sudo apt-get update -y
RUN sudo apt-get install -y x2goserver x2goserver-xsession

# Install MATE Desktop
RUN sudo apt-add-repository ppa:ubuntu-mate-dev/ppa -y && sudo apt-add-repository ppa:ubuntu-mate-dev/trusty-mate -y
RUN sudo apt-get update && sudo apt-get upgrade -y
RUN sudo apt-get install -y mate-desktop-environment-core && sudo apt-get install -y x2gomatebindings
RUN sudo apt-get install -y wget vim htop git curl unzip

# Remove no-use data
RUN sudo rm -rf /var/lib/apt/lists/*

# Set User Passwordf
RUN echo "gesture:foxconn" | chpasswd

# Set Environment Variable
RUN echo "export CUDA_HOME=/usr/local/cuda" >> /etc/bash.bashrc
RUN echo "export PATH=/usr/local/bin:/usr/local/nvidia/bin:/usr/local/nvidia/sbin:/usr/local/bin:/usr/local/sbin:/usr/sbin:/usr/bin:/sbin:/bin" >> /etc/bash.bashrc
RUN echo "export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/usr/local/nvidia/lib:/usr/local/nvidia/lib64:/usr/lib:/usr/local/lib" >> /etc/bash.bashrc

# Set SSH Config
RUN mkdir -p /var/run/sshd

EXPOSE 22

# Start sshd
CMD /usr/sbin/sshd && /bin/bash
