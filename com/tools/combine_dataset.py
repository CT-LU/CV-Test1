# A Dataset Combinator

import sys, os
import os.path as osp
from os.path import basename
import numpy as np
from xml.dom import minidom
import shutil as shu
import cv2

# Define Variable
ROOT_DIR = osp.split(osp.realpath(__file__))[0]

class dataset_combinator(object):
    def __init__(self, dataset_list):
        
        #dataset list
        self._dataset_list = dataset_list
        # trainval
        self._trainval_path = osp.join(ROOT_DIR, 'output', 'all_HandDataset', 'ImageSets', 'Main')
        if not osp.exists(self._trainval_path):
			#Create File if not exist
			os.makedirs(self._trainval_path)
        # trainval file
        self._trainval_file = osp.join(self._trainval_path, 'trainval.txt')

        # Dataset Image Path
        self._image_path = osp.join(ROOT_DIR, 'output', 'all_HandDataset', 'JPEGImages')
        if not osp.exists(self._image_path):
			#Create Folder if not exist
			os.makedirs(self._image_path)

        # Dataset Annotation Path
        self._annotation_path = osp.join(ROOT_DIR, 'output', 'all_HandDataset', 'Annotations')
        if not osp.exists(self._annotation_path):
			#Create Folder if not exist
			os.makedirs(self._annotation_path)


        #image file type
        self._image_ext = '.jpg'
        
        #Check file exist
        assert osp.exists(self._trainval_path), \
                'Dataset trainval.txt does not exist: {}'.format(self._trainval_path)
        assert osp.exists(self._image_path), \
                'Image Path does not exist: {}'.format(self._image_path)
        assert osp.exists(self._annotation_path), \
                'Annotation Path does not exist: {}'.format(self._annotation_path)

    def get_dataset_trainval_file(self, dataset_name):
        trainval = osp.join(ROOT_DIR, dataset_name, 'ImageSets', 'Main', 'trainval.txt')
        assert osp.exists(trainval), \
                'Dataset {} -> trainval.txt does not exist: {}'.format(dataset_name, trainval)
        return trainval

    def get_dataset_image_path(self, dataset_name):
        image_path = osp.join(ROOT_DIR, dataset_name, 'JPEGImages')
        assert osp.exists(image_path), \
                'Dataset {} -> Image Path does not exist: {}'.format(dataset_name, image_path)
        return image_path

    def get_dataset_annotation_path(self, dataset_name):
        annotation_path = osp.join(ROOT_DIR, dataset_name, 'Annotations')
        assert osp.exists(annotation_path), \
                'Dataset {} -> Annotation Path does not exist: {}'.format(dataset_name, annotation_path)
        return annotation_path

    def combine(self):

        #Combine Dataset
        output_image_list = []
        for dataset_name in self._dataset_list:
            #Get Image list
            image_list = [line.strip() for line in open(self.get_dataset_trainval_file(dataset_name), 'r')]
            #print 'image_list: {}'.format(image_list)
            image_path = self.get_dataset_image_path(dataset_name)
            annotation_path = self.get_dataset_annotation_path(dataset_name)

            #Read Image by filename
            for image_name in image_list:
                #Copy Dataset
                image = osp.join(image_path, image_name + self._image_ext)
                #print 'Read Image from: {}'.format(image_path)
                #Read Annotation file
                annotation = osp.join(annotation_path, image_name + '.xml')
                #print 'Read Annotation from: {}'.format(annotation_path)
                
                #Ouput Path
                output_image = osp.join(self._image_path, image_name + self._image_ext)
                output_annotation = osp.join(self._annotation_path, image_name + '.xml')
                
                #Copy
                shu.copy(image, output_image)
                shu.copy(annotation, output_annotation)

                #Append image_name to trainval.txt
                output_image_list.append(image_name)

            
        #Save new trainval.txt for all Datasets
        if output_image_list:
            #Save new trainval.txt
            with open(self._trainval_file, 'w+') as trainval:
                for output_image_name in output_image_list:
                    #Write image name to new trainval
                    trainval.write(output_image_name + '\n')

if __name__ == '__main__':
    print 'Combine Datasets:'
    for arg in sys.argv[1:]:
        print 'dataset -> {}'.format(arg)

    combinator = dataset_combinator(sys.argv[1:])
    # combine datasets
    result = combinator.combine()




