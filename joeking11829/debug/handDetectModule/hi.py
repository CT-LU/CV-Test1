import sys
import cv2
import numpy as np

class Ludan():
    img = None # keep image data from C langauge

    '''
    C invokes the sendImge() to send a image from C to Python
    '''
    def sendImg(self, frame, height, width, channels):
        self.img = np.frombuffer(frame, np.uint8)
        self.img = np.reshape(self.img, (height, width, channels))
        print self.img.shape
        
        return 110, 50, 600, 50

    '''
    C invokes the showImge() to show a image which has been sent by sendImg 
    '''
    def showImg(self):
        print self.img.shape

        while True:
            cv2.imshow('showImg', self.img)
            
            k = 0xFF & cv2.waitKey(30) 
            # key bindings
            if k == 27:         # esc to exit
                break
            
        return 'Hi Ludan'

