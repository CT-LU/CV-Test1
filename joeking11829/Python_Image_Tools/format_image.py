# A Image Formatter

import sys, os
import os.path as osp
from os.path import basename
import numpy as np
from xml.dom import minidom
import shutil as shu
import cv2

# Define Variable
#ROOT_DIR = osp.join(osp.dirname(__file__), '..', '..')
ROOT_DIR = osp.split(osp.realpath(__file__))[0]

#print ROOT_DIR

class image_formatter(object):
	def __init__(self, folder_path, devkit_path=None):
		
		self._devkit_path = self._get_default_path() if devkit_path is None \
							else devkit_path

		#Data Path
		self._data_path = osp.join(self._devkit_path, folder_path)
		#self.target_classes = ('person')

		#Result Path
		#Annotation
		self._result_annotation_path = osp.join(ROOT_DIR, 'result', 'HandDataset', 'Annotations')
		if not osp.exists(self._result_annotation_path):
			#Create Result Folder if not exist
			os.makedirs(self._result_annotation_path)
		#Image
		self._result_image_path = osp.join(ROOT_DIR, 'result', 'HandDataset', 'JPEGImages')
		#Check Result Path is exist
		if not osp.exists(self._result_image_path):
			#Create Result Folder if not exist
			os.makedirs(self._result_image_path)
		#File list
		self._result_list_path = osp.join(ROOT_DIR, 'result', 'HandDataset', 'ImageSets', 'Main')
		#Check Result Path is exist
		if not osp.exists(self._result_list_path):
			#Create Result Folder if not exist
			os.makedirs(self._result_list_path)


		#image file type
		self._image_ext = '.jpg'
		
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

	def formatImage(self):
		"""
		Find the target classes subset in current dataset
		"""
		
		print 'Image Formating ......'
		print 'Data Path: {}'.format(self._data_path)
		#print 'File list: {}'.format(os.listdir(self._data_path))

		#the Image file list
		image_files = [ osp.join(self._data_path, f) for f in os.listdir(self._data_path) if osp.isfile(osp.join(self._data_path, f)) ]
		#print image_files

		for image in image_files:
			print 'Image Path: {}'.format(image)
			img = cv2.imread(image)

			#Original Image
			#cv2.imshow('Original Image', img)

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
			#cv2.imshow('Resize Image', resize_img)
			#cv2.waitKey(0)

			output_image_path = self._result_image_path + '/' + basename(osp.splitext(image)[0] + '.jpg')
			print 'Output Path: {}'.format(output_image_path)
			cv2.imwrite(output_image_path, resize_img)
		
		#Release OpenCV Object
		cv2.destroyAllWindows()


if __name__ == '__main__':
	image_folder = sys.argv[1]
	print image_folder

	formatter = image_formatter(image_folder)
	# format image
	result = formatter.formatImage()

