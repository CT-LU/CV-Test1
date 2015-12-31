# A Dataset Validater

import sys, os
import os.path as osp
from os.path import basename
import numpy as np
from xml.dom import minidom
import shutil as shu
import cv2

# Define Variable
ROOT_DIR = osp.split(osp.realpath(__file__))[0]

class dataset_validater(object):
    def __init__(self, dataset_name):

        self._dataset_name = dataset_name
        self._dataset_path = osp.join(ROOT_DIR, self._dataset_name)

        # trainval file
        self._trainval_file = osp.join(ROOT_DIR, self._dataset_name, 'ImageSets', 'Main', 'trainval.txt')

        # Dataset Image Path
        self._image_path = osp.join(ROOT_DIR, self._dataset_name, 'JPEGImages')

        # Dataset Annotation Path
        self._annotation_path = osp.join(ROOT_DIR, self._dataset_name, 'Annotations')

        #image file type
        self._image_ext = '.jpg'

        #Check file exist
        assert osp.exists(self._trainval_file), \
                'Dataset trainval.txt does not exist: {}'.format(self._trainval_file)
        assert osp.exists(self._image_path), \
                'Image Path does not exist: {}'.format(self._image_path)
        assert osp.exists(self._annotation_path), \
                'Annotation Path does not exist: {}'.format(self._annotation_path)

    def validate(self):

        #Define method for get tag
        def get_data_from_tag(node, tag):
            return node.getElementsByTagName(tag)[0].childNodes[0].data

        def get_value_from_tag(node, tag):
            return node.getElementsByTagName(tag)[0].firstChild.nodeValue

        #Validate input Dataset
        #Get Image list
        image_list = [line.strip() for line in open(self._trainval_file, 'r')]
        print 'image_list: {}'.format(image_list)
        
        #Store image name to delete
        delete_image_list = []

        #Read Image by filename
        for image_name in image_list:
            #Using OpenCV to Read Image
            image_path = osp.join(self._image_path, image_name + self._image_ext)
            print 'Read Image from: {}'.format(image_path)

            #Read Image
            input_image = cv2.imread(image_path)
            original_image = input_image.copy()

            #Create Two Image View
            #img_height, img_width, img_channels = input_image.shape
            #result_two_image = np.zeros((img_height, (img_width*2), img_channels), np.uint8)
            #show the Original Image
            #cv2.imshow('Read Image', input_image)

            #Read Annotation file
            annotation_path = osp.join(self._annotation_path, image_name + '.xml')
            print 'Read Annotation from: {}'.format(annotation_path)
            with open(annotation_path) as f:
                annotation_data = minidom.parseString(f.read())
            
            # get 'object' elements
            objects = annotation_data.getElementsByTagName('object')

            """
            for obj in objects:
                print 'DOM: {}'.format(obj.toprettyxml())
            """

            num_objects = len(objects)

            #Draw Rectangle for each 'object' element
            for annotation_obj in objects:
                #get xmin, ymin, xmax, ymax
                class_name = str(get_value_from_tag(annotation_obj, 'name'))
                xmin = int(get_value_from_tag(annotation_obj, 'xmin'))
                ymin = int(get_value_from_tag(annotation_obj, 'ymin'))
                xmax = int(get_value_from_tag(annotation_obj, 'xmax'))
                ymax = int(get_value_from_tag(annotation_obj, 'ymax'))
                print 'object {} on -> xmin: {}, ymin: {}, xmax: {}, ymax: {}'.format(class_name, xmin, ymin, xmax, ymax)
                #Draw Rectangle for the class
                cv2.rectangle(input_image, (xmin, ymin), (xmax, ymax), (0,255,0), 2)
                #Draw the class name on Rectangle
                font = cv2.FONT_HERSHEY_SIMPLEX
                font_scale = 0.65
                text_size, text_baseline = cv2.getTextSize(class_name, font, font_scale, 2)
                print 'TextSize: {}  Text_Baseline: {}'.format(text_size, text_baseline)
                cv2.putText(input_image, class_name, (xmin, ymin+text_size[1]), font, font_scale, (0,0,255), 2, bottomLeftOrigin=False)

            #Show the Result Image
            #cv2.imshow('Result Image', input_image)
            #Combine two image to the Result
            result_two_image = np.hstack((original_image, input_image))
            cv2.imshow('Result Image', result_two_image)

            key_code = cv2.waitKey(0)
            #Add OpenCV Key Control Flow
            if key_code == ord('n'):
                continue
            elif key_code == ord('d'):
                #Delete current data
                try:
                    #Remove Image
                    print 'Remove Image: {}'.format(image_path)
                    os.remove(image_path)
                    #Remove Annotation
                    print 'Remove Annotation: {}'.format(annotation_path)
                    os.remove(annotation_path)
                except OSError:
                    pass
                #Remove filename in trainval.txt
                delete_image_list.append(image_name)
            elif key_code == 27:
                print 'Terminate the Program !!'
                break
            #...................................

        #Save new trainval.txt after delete images
        if delete_image_list:
            #Delete image exists
            #Rename Old trainval.txt to trainval.txt.backup
            os.rename(self._trainval_file, self._trainval_file + '.backup')
            #Save new trainval.txt without image names which are deleted
            with open(self._trainval_file + '.backup', 'r') as old_trainval, open(self._trainval_file, 'w+') as new_trainval:
                #filter
                for image_name in old_trainval:
                    if not any(delete_image in image_name for delete_image in delete_image_list):
                        #Write image name to new trainval
                        new_trainval.write(image_name)
        

        #Release OpenCV Object
        cv2.destroyAllWindows()

if __name__ == '__main__':
    dataset_name = sys.argv[1]
    print 'Dataset: {}'.format(dataset_name)

    validater = dataset_validater(dataset_name)
    # validate image
    result = validater.validate()




