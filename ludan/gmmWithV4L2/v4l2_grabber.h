#ifndef __V4L2_GRABBER_H
#define __V4L2_GRABBER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/videodev2.h>
#include <libv4l2.h>


#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define SUCCESS 0
#define FAILED -1
#define ERR_NOTSUPPORTED -2
//***Camera constants***
#define CAMERA_NO 1
#define CAM0_DEFAULT_RES RES_1920x1080	  //camera0 default resultion 1920x1080
#define CAM1_DEFAULT_RES RES_1920x1080	  //camera1 default resultion 1920x1080
#define CAM0_MASK 0x00000001
#define CAM1_MASK 0x00000010
enum SUPPORT_RESOLUTION { //supported resoultions
	RES_160x120,
	RES_176x144,
	RES_320x240,
	RES_352x288,
	RES_640x480,
	RES_800x600,
	RES_1280x720,
	RES_1920x1080
};
enum PIXEL_FOMRAT { //Supported pixel formats
	YUYV,     
	RGB24
};	
//Pixel format: V4L2_PIX_FMT_RGB24
//Byte 0 in memory	 	     Byte 1	 	         Byte 2
//_______________________    _______________________    _______________________
//r7 r6 r5 r4 r3 r2 r1 r0    g7 g6 g5 g4 g3 g2 g1 g0    b7 b6 b5 b4 b3 b2 b1 b0
#define CAM0_DEF_PIXELFORMAT RGB24   //camera0 default pixel format is RGB24
#define CAM1_DEF_PIXELFORMAT RGB24   //camera1 default pixel format is RGB24
#define CAM0_DEF_DEVICE "/dev/video0"
#define CAM1_DEF_DEVICE "/dev/video1"
//Camera types
enum CAM_TYPE {
	DUAL_CAMERA,   //dual camera: F100*2
	ETRON_CAMERA	//ToDo: Etron StereoCamera eSP870
};
//***Camera structures***
//Resolution
struct grab_resolution {
	__u32                   width;
        __u32                   height;
};
//The buffer for caller
struct buffer {
        int   *start;
        size_t length;
};
//The buffer for keeping retrieve
struct retrieve_buffer {
        void   *start;
        size_t length;
};
struct camera_settings {
	int res_index; //the index in support list of resolutions
	int pixelfmt_index; //the index in support list of Pixel formats
};


//***Functions***
/*
 *int initGrabber(int camera_type)
 *    Initialize the v4l2 grabber. It will open the device nodes and configure the camera with default resolution&pixel format.
 *    Note. Default resolution is 1920x1080, pixelformat is V4L2_PIX_FMT_RGB24
 *    Parameters:
 *     type: The type of camera, e.g. DUAL_CAMERA for daul camera(F100*2)
 *    Returns:
 *     return SUCCESS if initialized successfully, return FAILED if failed to initialize
 */
int initGrabber(int type);
/*
 *int shutdownGrabber(void)
 *    Shutdown the v4l2 grabber. It will stop streaming(VIDIOC_STREAMOFF) and then close the device nodes.
 *    Returns:
 *     return SUCCESS if shutdown successfully, return FAILED if failed to shutdown
 */
int shutdownGrabber(void);
/*
 *int configGrabber(struct camera_settings settings[]);
 *    Configure the v4l2 grabber. It will setup the image format(VIDIOC_S_FMT).
 *      NOTE. Optional for initialization, Please invoke it only when streming is stopped.
 *    Parameters:
 *     1.settings: The Camera streaming format settings, inculding the resolution and pixel format
 *     2.index: specify which camera will be configured, index: 0 to n integer
 *    Returns:
 *     return SUCCESS if configured successfully, return FAILED if failed to configure
 */
int configGrabber(struct camera_settings settings, int index);
/*
 *int startGrabber(void);
 *    Start the streaming. It will request the capture buffer(VIDIOC_REQBUFS) and  start streaming(VIDIOC_STREAMON).
 *    Returns:
 *     return SUCCESS if started successfully, return FAILED if failed to start
 */
int startGrabber(void);
/*
 *int stopGrabber(void);
 *    Stop the streaming. It will stop streaming(VIDIOC_STREAMOFF).
 *    Returns:
 *     return SUCCESS if stopped successfully, return FAILED if failed to stop
 */
int stopGrabber(void);
/*
 *int retrieveImage(struct buffer *beffers[]);
 *    Retrieve images from cameras to the integer arrays in memory.
 *Original format: V4L2_PIX_FMT_RGB24
 *Byte 0 in memory	 	     Byte 1	 	         Byte 2
 *_______________________    _______________________    _______________________
 *r7 r6 r5 r4 r3 r2 r1 r0    g7 g6 g5 g4 g3 g2 g1 g0    b7 b6 b5 b4 b3 b2 b1 b0
 *Transformed format:
 *Byte 0 in memory	 	     Byte 1	 	         Byte 2			Byte 3(Left w/o change)
 *_______________________    _______________________    ________________________	______________________
 *r7 r6 r5 r4 r3 r2 r1 r0    g7 g6 g5 g4 g3 g2 g1 g0    b7 b6 b5 b4 b3 b2 b1 b0		X X X X X X X X
 *
 *    Parameters:
 *     beffers: The array of image buffers and they are allocated by caller.
 *		buffers[0] for /dev/video0, buffers[1] for /dev/video1
 *    Returns:
 *     return SUCCESS if configured successfully, return FAILED if failed to configure
 */
int retrieveImage(struct buffer beffers[]);
/*
 *int saveImage(const char *ppm0, const char *ppm1)
 *    Save the images from cameras as ppm files.
 *    Parameters:
 *     ppm0: The absolute path of ppm file to save the image from /dev/video0
 *     ppm1: The absolute path of ppm file to save the image from /dev/video1
 *    Returns:
 *     return SUCCESS if saved successfully, return FAILED if failed to save
 */
int saveImage(const char *ppm0, const char *ppm1);

#ifdef __cplusplus
}
#endif
#endif
