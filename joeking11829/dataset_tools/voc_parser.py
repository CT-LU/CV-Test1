# A VOCdevkit Dataset parser

import sys, os
import os.path as osp
import numpy as np
from xml.dom import minidom
import shutil as shu

# Define Variable
#ROOT_DIR = osp.join(osp.dirname(__file__), '..', '..')
ROOT_DIR = osp.split(osp.realpath(__file__))[0]

#print ROOT_DIR

class pascal_voc_parser(object):
	def __init__(self, image_set, year, devkit_path=None):
		self._year = year
		self._image_set = image_set
		self._devkit_path = self._get_default_path() if devkit_path is None \
							else devkit_path

		#Data Path
		self._data_path = osp.join(self._devkit_path, 'VOC2007')
		#self.target_classes = ('person')

		#Result Path
		#Annotation
		self._result_annotation_path = osp.join(ROOT_DIR, 'result', 'VOC2007', 'Annotations')
		if not osp.exists(self._result_annotation_path):
			#Create Result Folder if not exist
			os.makedirs(self._result_annotation_path)
		#Image
		self._result_image_path = osp.join(ROOT_DIR, 'result', 'VOC2007', 'JPEGImages')
		#Check Result Path is exist
		if not osp.exists(self._result_image_path):
			#Create Result Folder if not exist
			os.makedirs(self._result_image_path)
		#File list
		self._result_list_path = osp.join(ROOT_DIR, 'result', 'VOC2007', 'ImageSets', 'Main')
		#Check Result Path is exist
		if not osp.exists(self._result_list_path):
			#Create Result Folder if not exist
			os.makedirs(self._result_list_path)


		#image file type
		self._image_ext = '.jpg'
		
		#load image index from trainval.txt
		self._image_index = self._load_image_set_index()
		
		# PASCAL specific config options
		self.config = {'use_diff' : False}

		#Check file exist
		assert osp.exists(self._devkit_path), \
				'VOCdevkit path does not exist: {}'.format(self._devkit_path)
		assert osp.exists(self._data_path), \
				'Path does not exist: {}'.format(self._data_path)

	def _get_default_path(self):
		"""
		Return the default file path
		"""
		return osp.join(ROOT_DIR, 'VOCdevkit2007')

	def _load_image_set_index(self):
		"""
		Load the indexes listed in this dataset's image set file
		"""
		# Example path to image set file:
		# self_devkit_path + /VOCdevkit2007/VOC2007/ImageSets/trainval.txt
		image_set_file = osp.join(self._data_path, 'ImageSets', 'Main', self._image_set + '.txt')

		# Check file path exist !
		assert osp.exists(image_set_file), \
				'Path does not exist: {}'.format(image_set_file)
		
		# Get index form file
		with open(image_set_file) as f:
			image_index = [x.strip() for x in f.readlines()]

		return image_index

	def extract_target_classes(self):
		"""
		Find the target classes subset in current dataset
		"""

		#Define method for get tag
		def get_data_from_tag(node, tag):
			return node.getElementsByTagName(tag)[0].childNodes[0].data
		
		#Debug for Load index file
		#print '_image_index: {}'.format(self._image_index)

		#the New list file
		list_file_handler = open(osp.join(self._result_list_path,  'trainval.txt'), 'wb')

		for index in self._image_index:
			# Load Annotation file
			filename = osp.join(self._data_path, 'Annotations', index + '.xml')
			print 'Loading: {}'.format(filename)
			
			# Read Annotation file
			with open(filename) as f:
				data = minidom.parseString(f.read())

			# get 'object' elemets
			objs = data.getElementsByTagName('object')
			
			"""
			for obj in objs:
				print 'DOM: {}'.format(obj.toprettyxml())
			"""

			# Remove difficult object
			if not self.config['use_diff']:
				# Exclude the samples labeled as difficult
				non_diff_objs = [obj for obj in objs
								if int(get_data_from_tag(obj, 'difficult')) == 0]
				if len(non_diff_objs) != len(objs):
					print 'Removed {} difficult objects' \
						.format(len(objs) - len(non_diff_objs))
				objs = non_diff_objs

			# Remove non-person object
			person_objs = [obj for obj in objs
							if(get_data_from_tag(obj, 'name')) == 'person']
			objs = person_objs

			# number of objs
			num_objs = len(objs)

			# Must more than one person object then copy the image and create new annotation file to result folder
			if int(num_objs) > 0:
				print '{} image exist person object'.format(index)
				
				# 1. Write index to the New list file
				list_file_handler.write(index + '\n')

				# 2. Copy current image to result folder
				image_filename = osp.join(self._data_path, 'JPEGImages', index + self._image_ext)
				# Check image file path exist !
				assert osp.exists(image_filename), \
				'Image Path does not exist: {}'.format(image_filename)
				# Copy Image
				shu.copy(image_filename, self._result_image_path)
				
				# 3. Create new annotaion for current image to mark person object
				"""
				for obj in objs:
					print 'DOM: {}'.format(obj.toprettyxml())
				"""
				
				root = data.documentElement
				for r_obj in data.getElementsByTagName('object'):
					print 'Remove Element: {}'.format(r_obj)
					root.removeChild(r_obj)
				
				for obj in objs:
					print 'Add Element: {}'.format(obj)
					root.appendChild(obj)

				# Format xml
				xmlStr = data.toprettyxml(indent = '', newl = '', encoding = 'utf-8')
				xmlStr = xmlStr.replace('\t', '').replace('\n', '')
				data = minidom.parseString(xmlStr)
				data = minidom.parseString(data.toprettyxml(indent = '\t', newl = '\n', encoding = 'utf-8'))
				root = data.documentElement
				#print 'data DOM: {}'.format(data.toprettyxml())
				
				# Create new Annotation xml file
				file_handler = open(osp.join(self._result_annotation_path, index + '.xml'), 'wb')
				root.writexml(file_handler)
				file_handler.close()

		#Close the New list file
		list_file_handler.close()


if __name__ == '__main__':
	parser = pascal_voc_parser('trainval', '2007')
	# extract person dataset
	result = parser.extract_target_classes()


