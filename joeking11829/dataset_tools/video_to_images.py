# A Image Formatter

import sys, os
import os.path as osp
from os.path import basename
import numpy as np
from xml.dom import minidom
import shutil as shu
import cv2

import datetime
import random

# Define Variable
#ROOT_DIR = osp.join(osp.dirname(__file__), '..', '..')
ROOT_DIR = osp.split(osp.realpath(__file__))[0]

#print ROOT_DIR

#Global Variables
frame = []
clone_frame = []
startPt =[]
endPt = []
cropping = False
#Create namedWindow
cv2.namedWindow('Input_Image')


class video_to_images(object):
    def __init__(self, video_filename, devkit_path=None):
        
        self._video_filename = osp.join(ROOT_DIR, video_filename)

        self._devkit_path = self._get_default_path() if devkit_path is None \
                            else devkit_path

        #Data Path
        self._data_path = self._devkit_path

        #Image
        self._output_images_path = osp.join(ROOT_DIR, 'output', 'JPEGImages')
        #Check Result Path is exist
        if not osp.exists(self._output_images_path):
            #Create Result Folder if not exist
            os.makedirs(self._output_images_path)

        
        #Check file exist
        assert osp.exists(self._devkit_path), \
                'HandDataset path does not exist: {}'.format(self._devkit_path)
        assert osp.exists(self._data_path), \
                'Path does not exist: {}'.format(self._data_path)

    def _get_default_path(self):
        """
        Return the default file path
        """
        return osp.join(ROOT_DIR)

    def _click_and_crop(self, event, x, y, flags, param):
        #grab references to the global variables
        global startPt, endPt, cropping, frame, clone_frame

        #Handle Mouse Event
        if event == cv2.EVENT_LBUTTONDOWN:
            startPt = (x,y)
            cropping = True

        elif event == cv2.EVENT_LBUTTONUP:
            endPt = (x,y)
            cropping = False
            #cv2.rectangle(frame, startPt, endPt, (0, 255, 0), 2)
            print 'Crop Rect -> start_x: {}, start_y: {}, end_x: {}, end_y: {}'.format(startPt[0], startPt[1], endPt[0], endPt[1])
            print 'Crop Rect -> width: {}, height: {}'.format(endPt[0]-startPt[1], endPt[1]-startPt[1])
            frame = clone_frame[startPt[1]:endPt[1], startPt[0]:endPt[0]]
            cv2.imshow('Input_Image', frame)
        elif event == cv2.EVENT_MOUSEMOVE and cropping == True:
            tempPt = (x,y)
            frame = clone_frame.copy()
            cv2.rectangle(frame, startPt, tempPt, (0, 255, 0), 2)
            print 'Temp Rect -> width: {}, height: {}'.format(tempPt[0]-startPt[1], tempPt[1]-startPt[1])
            cv2.imshow('Input_Image', frame)


    def captureImages(self):
        
        global frame, clone_frame

        print 'Capture Image ......'
        #print 'Data Path: {}'.format(self._data_path)

        #the Video filename
        print self._video_filename
        #Open Video File
        video_cap = cv2.VideoCapture(self._video_filename)

        #Set Mouse Callback
        cv2.setMouseCallback('Input_Image', self._click_and_crop)
               
        #Read the first frame of mp4 file
        if video_cap.isOpened() == True:
            ret, frame = video_cap.read()
            clone_frame = frame.copy()

        while(video_cap.isOpened()):
            
            #print 'Image Raw Data: {}'.format(frame)
            cv2.imshow('Input_Image', frame)
            #cv2.imshow('Clone Image', clone_frame)
            
            #Key Code
            key_code = cv2.waitKey(0)
            #Add OpenCV Key Control Flow
            if key_code == ord('n'):
                #Skip the current image
                print 'Go to Next Image !!'
                #Read the next image
                ret, frame = video_cap.read()
                clone_frame = frame.copy()
                continue
            elif key_code == ord('r'):
                #Recover Image
                print 'Recover the Image !!'
                frame = clone_frame.copy()
                continue
            elif key_code == ord('s'):
                #Save the current image
                print 'Save the Image'
                #Generate Unique Name
                nowTime = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
                randomNum = random.randint(0, 1000)
                uniqueName = str(nowTime) + str(randomNum)

                output_image_path = osp.join(self._output_images_path, uniqueName + '_hand.jpg')
                print 'Output Image Path: {}'.format(output_image_path)
                cv2.imwrite(output_image_path, frame)
                #Read the next image
                ret, frame = video_cap.read()
                clone_frame = frame.copy()
                continue
            elif key_code == 27:
                print 'Terminate the Program !!'
                break
            
            """
            #Resize image if (width or height) > 500
            scale = 0
            img_height, img_width, img_channels = img.shape
            if img_width > img_height:
                if img_width > 500:
                    #resize img_width to 500
                    scale = float(500)/img_width
                    #print scale, ' ', img_width
            else:
                if img_height > 500:
                    #resize img_height to 500
                    scale = float(500)/img_height
                    #print scale, ' ', img_height

            #resize image
            #print 'img_width: {}'.format(img_width)
            #print 'img_height: {}'.format(img_height)
            #print 'scale: {}'.format(scale)
            resize_img = cv2.resize(img, (int(img_width*scale), int(img_height*scale)))
            """
        #Release OpenCV Object
        cv2.destroyAllWindows()


if __name__ == '__main__':
    video_name = sys.argv[1]
    print video_name

    captr = video_to_images(video_name)
    # capture image
    result = captr.captureImages()

