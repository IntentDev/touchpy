import keyboard # optional, used to quit the loop
import numpy as np
import torch

from image_filter import ImageFilter

import touchpy as tp

class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.device = torch.device('cuda')
		self.imag_filter = ImageFilter().to(self.device)

		# used to test InChopLink
		self.frame = 0
		self.test_array = np.array([[1],[2],[3],[4],[5],[6],[7],[8],[9],[10]], dtype=np.float32)
		self.test_array_chan_names = [f"chn{i}" for i in range(10)]

	@staticmethod
	def on_frame(comp, this):

		# optional used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			comp.stop()
			return

		# copy out_chop to in_chop with channel names. Only NumPy arrays are supported for now.
		arr = comp.out_chops[0].as_numpy()
		names = comp.out_chops[0].chan_names()
		arr *= 2
		comp.in_chops[0].from_numpy(arr, names)
		
		# set in_chops[0] with local data
		# comp.in_chops[0].from_numpy(this.test_array, this.test_array_chan_names)

		# update the local data
		this.test_array += .01

		chans2 = comp.out_chops[1].as_numpy()
		chans2_names = comp.out_chops[1].chan_names()
		# print(chans2_names)

		# retrieve by name and set as local variable
		in_chop2 = comp.in_chops['chopIn2']
		in_chop2.from_numpy(chans2, chans2_names)

		# # print some channel data
		chans3 = comp.out_chops[2]
		arr = chans3.as_numpy()
		# print(arr)
		comp.in_chops[2].from_numpy(arr, chans3.chan_names())

		# set first in DAT with string (inDAT will be in text mode)
		comp.in_dats[0].from_string(f"Hello World! frame: {this.frame}")
				
		# create a DatTable and fill it with a list of data
		datTable = tp.DatTable()
		testList = [['g', 'b', 'c'], ['g', 'h', 'i'], ['t', 'w', 'a']]
		datTable.from_list(testList)

		# set second in DAT with table
		# comp.in_dats['datIn2'].from_table(datTable)

		# set second in DAT with list
		comp.in_dats['datIn2'].from_list(testList)

		# print some outDat data
		datOut1 = comp.out_dats['datOut1']
		if (datOut1 is not None):
			# print(datOut1.as_string())
			# print(datOut1.as_table().row(0))
			# print(datOut1.as_table().row(1))
			# print(datOut1.as_table().col(0))
			# print(datOut1.as_table().cell(2,2))
			# print(datOut1.as_table().as_list())
			pass
			
		# print some outDat data
		datOut2 = comp.out_dats[1]
		if (datOut2 is not None):
			# print(datOut2.as_string())
			# print(datOut2.as_table().as_list())
			pass
		
		# print out all the parameters name on the first frame
		parNames = comp.par.names()
		if (this.frame == 0):
			for name in parNames:
				print(f"{name}: {comp.par[name].val}")
			
		# get the parameter named Rgba and set it's values in different ways
		# setting individual components is not ideal, but it's possible
		# better set .val to a new tp.Color object or use .set()
		rgba = comp.par['Rgba']
		rgba.r = .8
		rgba.g = rgba.g + .2
		# comp.par['Rgba'].val = tp.Color(0.1, 0.2, 0.3, .5)

		# get the parameter named Scale and set it's value
		scale = comp.par['Scale']
		scale.val = 0.0 + this.frame * 0.01
		# print(scale.val)

		# get the parameter named Translate and set it's value
		# .set() can take a list, tuple, individual values or the correct type for the parameter
		translate = comp.par['Translate']
		# translate.val = tp.Float3(11.1, 22.2, 33.3)
		translate.set(tp.Float3(11.1, 22.2, 33.1))
		translate.set([11.1, 22.2, 33.2])
		translate.set(11.1, 22.2, 33.5)


		# copy the cuda memory from out_top_link to in_top_link
		cudamem = comp.out_tops[0].cuda_memory()
		comp.in_tops[0].copy_cuda_memory(cudamem)

		# copy the cuda memory from out_top_link to in_top_link
		cudamem = comp.out_tops[1].cuda_memory()
		comp.in_tops[1].copy_cuda_memory(cudamem)

		with torch.no_grad():
			# tensor = comp.out_tops[0].as_tensor() # get the first top as a tensor
			# tensor = comp.out_tops[0].as_tensor(tp.ComponentMask.RGB) # get just the first 3 channels
			# tensor2 = tensor * 2 # do some work on the tensor
			# comp.in_tops[0].from_tensor(tensor2)

			tensor = comp.out_tops[2].as_tensor()
			if (this.frame == 2):
				print("tensor shape: ", tensor.shape, "tensor dtype: ", tensor.
				dtype, "tensor device: ", tensor.device, "tensor layout: ", tensor.layout, 
				"tensor strides: ", tensor.stride(), "tensor is_contiguous: ", tensor.is_contiguous())

			# filter tensor only works with 32bit float data (comp.out_tops[2] is 32bit float in this example)
			# filter expects (b, c, h, w) layout
			tensor2 = this.imag_filter(tensor.unsqueeze(0)).squeeze(0) 

			if (this.frame == 2):
				print("tensor2 shape: ", tensor2.shape, "tensor2 dtype: ", tensor2.dtype, 
				"tensor2 device: ", tensor2.device, "tensor2 layout: ", tensor2.layout, 
				"tensor2 strides: ", tensor2.stride(), "tensor2 is_contiguous: ", tensor2.is_contiguous())
			
			comp.in_tops[2].from_tensor(tensor2)
			pass

		# comp.start_next_frame() is only needed if comp.start(start_next_frame=False)
		# or comp.update(call_start_next_frame=False), they are both False by default
		#
		# this allows the user to control when the next frame starts and so they can do work on non-comp members
		# if comp.start(True) or comp.update(True) is called, start_next_frame() will be called automatically
		# and shouldn't be called here
		if comp.start_next_frame():
			# do work on non-comp members for the next frame here
			pass
		else:
			# we have a problem with TouchEngine starting the next frame.
			# Currently the error will be printed in the console but start_next_frame()
			# will need to be updated to return an enum with the result for error handling
			# in the future
			pass

		this.frame += 1

	def runComp(self, tox_path):
		# create a comp object and specify a path to a tox file
		comp = tp.Comp(tox_path)

		# set the on_frame callback
		# first are is static method (or free function) and second is the object to be passed to the callback
		# in this case, self is passed to the callback so we can access the class members directly
		comp.set_on_frame_callback(self.on_frame, self)

	
		# this runs a loop that calls update() internally
		comp.start() # need to manually call start_next_frame() in on_frame callback to start next frame
		# comp.start(True) # update function starts next frame automatically

		# run the loop in Python anc call update manually
		# while not (keyboard.is_pressed('q')):
		# while self.running:
		#	# comp.update(True) # update function starts next frame
		# 	comp.update() #  need to manually call start_next_frame() in on_frame callback to start next frame
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('TopChopDatIO.tox')

