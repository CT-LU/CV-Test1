#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <libgen.h>
#include "v4l2_grabber.h"
#ifdef __cplusplus
extern "C" {
#endif
/*****camera settings*******/
//1.Device nodes
/*
char *camera_nodes[CAMERA_NO]={
	CAM0_DEF_DEVICE,
	CAM1_DEF_DEVICE };
*/
char *camera_nodes[CAMERA_NO]={
	CAM0_DEF_DEVICE };
//2.supported resolutions
struct grab_resolution res_list[]={
	{160, 120},
	{176, 144},
	{320, 240},
	{352, 288},
	{640, 480},
	{800, 600},
	{1280, 720},
	{1920, 1080}
};
//3.supported pixel formats
__u32 grab_pxlfmt[]={
	V4L2_PIX_FMT_YUYV,
	V4L2_PIX_FMT_RGB24
};
//fd of the cameras
int fd[CAMERA_NO] = {-1};
//int fd[CAMERA_NO] = { -1, -1};
//Capture format of the cameras
struct v4l2_format fmt[CAMERA_NO];
//Retrieved buffers
struct retrieve_buffer buffers[CAMERA_NO];   //mmap to driver
struct v4l2_buffer     buf[CAMERA_NO];
#define DEBUG 0


static void xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = v4l2_ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1) {
                fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}
int configGrabber(struct camera_settings settings, int index) {
	if(index >= CAMERA_NO) {
		fprintf(stderr, "The index(%d) exceeds the max index(%d)\n", index, CAMERA_NO);
		return FAILED;
	}
	if(fd[index]==-1) {
		fd[index] = v4l2_open(camera_nodes[index], O_RDWR | O_NONBLOCK, 0);
		if(fd[index] < 0) {
			fprintf(stderr, "Cannot open device %s", camera_nodes[index]);			
			return FAILED;
		}
	}
	CLEAR(fmt[index]);
	fmt[index].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt[index].fmt.pix.width = res_list[settings.res_index].width;
	fmt[index].fmt.pix.height = res_list[settings.res_index].height;
	//fmt[index].fmt.pix.pixelformat = grab_pxlfmt[settings.pixelfmt_index];
	fmt[index].fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt[index].fmt.pix.field       = V4L2_FIELD_INTERLACED;
        xioctl(fd[index], VIDIOC_S_FMT, &fmt[index]);
	return SUCCESS;
}
int initGrabber(int type) {
	int i=0;
	struct camera_settings the_settings;

	switch(type) {
	case DUAL_CAMERA:  //Genius F100*2
		for(i=0;i<CAMERA_NO;i++) {
			fd[i] = v4l2_open(camera_nodes[i], O_RDWR | O_NONBLOCK, 0);
			if (fd[i] < 0) {
				fprintf(stderr, "Cannot open device %s", camera_nodes[i]);
				shutdownGrabber();
				return FAILED;
			}
#if DEBUG
			printf("Opened device node of camera%d\n", i);
#endif
			//Configure the video streaming format
			if(i == 0) {
				the_settings.res_index = CAM0_DEFAULT_RES;
				the_settings.pixelfmt_index = CAM0_DEF_PIXELFORMAT;
			}
			else {
				the_settings.res_index = CAM1_DEFAULT_RES;
				the_settings.pixelfmt_index = CAM1_DEF_PIXELFORMAT;
			}
			configGrabber(the_settings, i);
#if DEBUG
			printf("Configured camera%d\n", i);
#endif

			//initialize the retrieved buffer
			buffers[i].start=0x00;
			buffers[i].length=0;
		}
		break;
	case ETRON_CAMERA: //ToDo: support the Etron depth camera
		perror("Not support Etron camera yet\n");
		return ERR_NOTSUPPORTED;
	default:
		fprintf(stderr, "Unknown camera type:%d\n", type);
		return ERR_NOTSUPPORTED;
		break;
	}
	return SUCCESS;
}

int shutdownGrabber(void) {
	enum v4l2_buf_type type;
	int i=0;
		
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	for(i=0;i<CAMERA_NO;i++) { 
		if(fd[i] >= 0) {
			//1.stop streaming
        		xioctl(fd[i], VIDIOC_STREAMOFF, &type);
			//2.unmap the buffers
			if(buffers[i].start != 0x00)
				v4l2_munmap(buffers[i].start, buffers[i].length);
			//3.close the fd
			v4l2_close(fd[i]);
			fd[i]=-1;
		}
	}
	return SUCCESS;
}

int startGrabber(void) {
	struct v4l2_requestbuffers      req[CAMERA_NO];
        enum v4l2_buf_type              type;
	int i;

	for(i=0;i<CAMERA_NO;i++) { 
		CLEAR(req[i]);
        	req[i].count = 1;
        	req[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	req[i].memory = V4L2_MEMORY_MMAP;
        	xioctl(fd[i], VIDIOC_REQBUFS, &req[i]);
#if DEBUG
			printf("requested buffer of camera%d\n", i);
#endif

        	CLEAR(buf[i]);
        	buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	buf[i].memory = V4L2_MEMORY_MMAP;
        	buf[i].index = 0;

        	xioctl(fd[i], VIDIOC_QUERYBUF, &buf[i]);
#if DEBUG
		printf("VIDIOC_QUERYBUF of camera%d\n", i);
#endif
        	buffers[i].length = buf[i].length;
        	buffers[i].start = v4l2_mmap(NULL, buf[i].length,
                              PROT_READ | PROT_WRITE, MAP_SHARED,
                              fd[i], buf[i].m.offset);

        	if (MAP_FAILED == buffers[i].start) {
        		fprintf(stderr, "mmap on camera%d", i);
			//ToDo: error handling
        		return FAILED;
        	}

		CLEAR(buf[i]);
                buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf[i].memory = V4L2_MEMORY_MMAP;
                buf[i].index = 0;
                xioctl(fd[i], VIDIOC_QBUF, &buf[i]);
#if DEBUG
		printf("VIDIOC_QBUF of camera%d\n", i);
#endif

		//Start streaming
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        	xioctl(fd[i], VIDIOC_STREAMON, &type);

#if DEBUG
		printf("Started streaming of camera%d\n", i);
#endif
	}
	return SUCCESS;
}
int stopGrabber(void) {
	enum v4l2_buf_type              type;
	int i;

	for(i=0;i<CAMERA_NO;i++) { 
		if(fd[i] != -1) {
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			xioctl(fd[i], VIDIOC_STREAMOFF, &type);
			if(buffers[i].start != 0x00)
		        	v4l2_munmap(buffers[i].start, buffers[i].length);
		
			v4l2_close(fd[i]);
			fd[i]=-1;
		}
	}

	return SUCCESS;
}
int copy2IamgeMem(void *start, unsigned int length,struct buffer data_buffer) {
	//Orignial format: V4L2_PIX_FMT_RGB24
	//Byte 0 in memory	 	     Byte 1	 	         Byte 2
	//_______________________    _______________________    _______________________
	//r7 r6 r5 r4 r3 r2 r1 r0    g7 g6 g5 g4 g3 g2 g1 g0    b7 b6 b5 b4 b3 b2 b1 b0
	//Transformed format in one integer
	//Byte 0 in memory	 	     Byte 1	 	         Byte 2	 	         Byte 3(not touched)
	//_______________________    _______________________    _______________________    _______________________
	//r7 r6 r5 r4 r3 r2 r1 r0    g7 g6 g5 g4 g3 g2 g1 g0    b7 b6 b5 b4 b3 b2 b1 b0    X7 X6 X5 X4 X3 X2 X1 X0
	void *source=start, *dest=(void *)data_buffer.start;
	int run_no=length/3; //how many run we shall do to copy RGB bytes? 
	int i=0;
	if(data_buffer.length < (length/3)*4) { //check whether the size of data_buffers is enough, 3bytes source for one integer
		fprintf(stderr, "The data_buffer length is not enough\n");
		return FAILED;
	}
	for(i=0; i<run_no; i++) { //copy 3 bytes(RGB) to data_buffers per time 
		memcpy(dest, (const void *)source, 3);
		source += 3; //jump to next pixel
		dest += 4; //jump to next integer 
	}
	return SUCCESS;
}
int retrieveImage(struct buffer data_buffers[]) {
	fd_set fds;
        struct timeval tv;
        int r, read_mask=CAM0_MASK, error_no=0, fd_no=0;
        //int r, read_mask=CAM0_MASK|CAM1_MASK, error_no=0, fd_no=0;
        do {
                FD_ZERO(&fds);
		if((read_mask&CAM0_MASK) == CAM0_MASK) {
			FD_SET(fd[0], &fds); //need to read Camera0
			fd_no++;
		}
		if((read_mask&CAM1_MASK) == CAM1_MASK) {
			FD_SET(fd[1], &fds); //need to read Camera1
			fd_no++;
		}
		if(fd_no<=0) { //no need to read any fd, break
			r=0;
			break;
		}
                /* Timeout. */
                tv.tv_sec = 1;
                tv.tv_usec = 0;
#if DEBUG
		printf("Waiting for frames comme in\n");
#endif
                r = select(fd[0] + 1, &fds, NULL, NULL, &tv);
                //r = select(fd[1] + 1, &fds, NULL, NULL, &tv);
		error_no = errno;
		if(r < 0) break;
		if(r > 0) {
			if(FD_ISSET(fd[0], &fds)) {
				CLEAR(buf[0]);
                		buf[0].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                		buf[0].memory = V4L2_MEMORY_MMAP;
                		xioctl(fd[0], VIDIOC_DQBUF, &buf);
				copy2IamgeMem(buffers[0].start, buf[0].bytesused,  data_buffers[0]);
				read_mask = read_mask&~CAM0_MASK;  //do not read Camera0 again				
				xioctl(fd[0], VIDIOC_QBUF, &buf[0]);
			}
			else  {
				CLEAR(buf[1]);
                		buf[1].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                		buf[1].memory = V4L2_MEMORY_MMAP;
                		xioctl(fd[1], VIDIOC_DQBUF, &buf[1]);
				copy2IamgeMem(buffers[1].start, buf[1].bytesused,  data_buffers[1]);
				read_mask = read_mask&~CAM1_MASK;  //do not read Camera1 again				
				xioctl(fd[1], VIDIOC_QBUF, &buf[1]);
			}
		}		
		fd_no=0;
        } while ((error_no = EINTR));
        if (r == -1) {
                perror("select");
		//ToDo: error handling
                return -1*errno;
        }
	return SUCCESS;	
}
int write2File(int index, unsigned int length,const char *file_name) {
	FILE *fout=NULL;
	if((fout=fopen(file_name, "w"))==NULL) {
		fprintf(stderr, "Unable to open the file:%s\n", file_name);
		return FAILED;
	}
	fprintf(fout, "P6\n%d %d 255\n",
                        fmt[index].fmt.pix.width, fmt[index].fmt.pix.height);
	fwrite(buffers[index].start, length, 1, fout);
	fflush(fout);
        fclose(fout);
	return SUCCESS;
}
int saveImage(const char *ppm0, const char *ppm1) {
	fd_set fds;
        struct timeval tv;
        int r, read_mask=CAM0_MASK|CAM1_MASK, error_no=0, fd_no=0;
        do {
                FD_ZERO(&fds);
		if((read_mask&CAM0_MASK) == CAM0_MASK) {
			FD_SET(fd[0], &fds); //need to read Camera0
			fd_no++;
		}
		if((read_mask&CAM1_MASK) == CAM1_MASK) {
			FD_SET(fd[1], &fds); //need to read Camera1
			fd_no++;
		}
		if(fd_no<=0) { //no need to read any fd, break
			r=0;
			break;
		}
                /* Timeout. */
                tv.tv_sec = 1;
                tv.tv_usec = 0;
		
                r = select(fd[0] + 1, &fds, NULL, NULL, &tv);
                //r = select(fd[1] + 1, &fds, NULL, NULL, &tv);
		error_no = errno;
		if(r < 0) break;
		if(r > 0) {
			if(FD_ISSET(fd[0], &fds)) {
				CLEAR(buf[0]);
                		buf[0].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                		buf[0].memory = V4L2_MEMORY_MMAP;
                		xioctl(fd[0], VIDIOC_DQBUF, &buf[0]);
#if DEBUG
				printf("Got a frame from Camera0\n");
#endif
				write2File(0, buf[0].bytesused, ppm0);
				read_mask = read_mask&~CAM0_MASK;  //do not read Camera0 again				
				xioctl(fd[0], VIDIOC_QBUF, &buf[0]);
			}
			else  {
				CLEAR(buf[1]);
                		buf[1].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                		buf[1].memory = V4L2_MEMORY_MMAP;
                		xioctl(fd[1], VIDIOC_DQBUF, &buf[1]);
#if DEBUG
				printf("Got a frame from Camera1\n");
#endif
				write2File(1, buf[1].bytesused,  ppm1);
				read_mask = read_mask&~CAM1_MASK;  //do not read Camera1 again
				xioctl(fd[1], VIDIOC_QBUF, &buf[1]);
			}
		}
		fd_no=0;
        } while ((error_no = EINTR));
        if (r == -1) {
                perror("select");
		//ToDo: error handling
                return -1*errno;
        }
	return SUCCESS;	
}
#ifdef __cplusplus
}
#endif
