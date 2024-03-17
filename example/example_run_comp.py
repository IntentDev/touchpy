import sys
import os
import keyboard
import numpy as np
import torch
from torch import nn

import touchpy as tp

class ImageFilter(nn.Module):
	"""
	Function to test io with TopLink tensors
	"""
	def __init__(self, in_channels=4, out_channels=4, kernel_size=3):
		super().__init__()

		# Create the convolutional layer
		self.conv = nn.Conv2d(in_channels, out_channels, kernel_size, stride=1, 
							  padding=kernel_size // 2, groups=4, bias=False)

		# Initialize the weights and biases
		nn.init.constant_(self.conv.weight, 1.0 / (kernel_size ** 2) )
		# nn.init.normal_(self.conv.weight, 0.0, 1)
		# nn.init.xavier_uniform_(self.conv.weight, gain=2.0)

	def forward(self, x):
		# Assuming x is of shape [batch_size, channels, height, width]
		return self.conv(x)

	def normalize(self, tensor):
		tensor_min = tensor.min()
		tensor_max = tensor.max()
		normalized_tensor = (tensor - tensor_min) / (tensor_max - tensor_min)
		return normalized_tensor


class ExampleRunComp:
	def __init__(self):
		self.frame = 0
		self.running = True
		self.test_array = np.array([[1],[2],[3],[4],[5],[6],[7],[8],[9],[10]], dtype=np.float32)
		self.test_array_chan_names = [f"chn{i}" for i in range(10)]

		self.device = torch.device('cuda')
		self.imag_filter = ImageFilter().to(self.device)

	def test(self, chans):
		print(chans)
		pass

	@staticmethod
	def on_frame(comp, this):

		# used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			comp.stop()
			return
		
		tensor = comp.out_tops[0].as_tensor()
		# tensor = comp.out_tops[0].as_tensor(tp.ComponentMask.RGB)

		# print("tensor shape: ", tensor.shape, "tensor dtype: ", tensor.
		# dtype, "tensor device: ", tensor.device, "tensor layout: ", tensor.layout, "tensor strides: ", tensor.stride(), "tensor is_contiguous: ", tensor.is_contiguous())

		with torch.no_grad():
			tensor2 = tensor * 2
			# tensor2 = tensor.clone()
			# tensor2 = tensor.permute(2, 1, 0).contiguous()

			# filter tensor only works with 32bit float data
			# tensor2 = this.imag_filter(tensor.unsqueeze(0)).squeeze(0) 

			# print("tensor2 shape: ", tensor2.shape, "tensor2 dtype: ", tensor2.dtype, "tensor2 device: ", tensor2.device, "tensor2 layout: ", tensor2.layout, "tensor2 strides: ", tensor2.stride(), "tensor2 is_contiguous: ", tensor2.is_contiguous())
			
			comp.in_tops[0].from_tensor(tensor2)
			pass


		cudamem = comp.out_tops[1].cuda_memory()
		comp.in_tops[1].copy_cuda_memory(cudamem)

		cudamem = comp.out_tops[2].cuda_memory()
		comp.in_tops[2].copy_cuda_memory(cudamem)

		comp.in_chops[0].from_numpy(comp.out_chops[0].as_numpy(), comp.out_chops[0].chan_names())
		# comp.in_chops[0].from_numpy(this.test_array, this.test_array_chan_names)
		this.test_array += .001

		# get a reference to the numpy array, some functions that do not copy 
		# will not work with this passed as an argument, such as ChopLink.from_numpy() 
		# faster than a copy though for large arrays... good for read only operations
		chans1_ref = comp.out_chops['chopOut1'].as_numpy_ref() 
		# comp.in_chops[1].from_numpy(chans1) # this will not work!

		chans2 = comp.out_chops[1].as_numpy()
		chans2_names = comp.out_chops[1].chan_names()
		# print(chans2_names)

		comp.in_chops[1].from_numpy(chans2, chans2_names)
		chans3 = comp.out_chops[2].as_numpy()
		# print(chans2)

		comp.in_dats[0].from_string(f"Hello World! frame: {this.frame}")
				
		datTable = tp.DatTable()
		testList = [['g', 'b', 'c'], ['g', 'h', 'i'], ['t', 'w', 'a']]
		datTable.from_list(testList)

		# comp.in_dats['datIn2'].from_table(datTable)
		comp.in_dats['datIn2'].from_list(testList)

		datOut1 = comp.out_dats['datOut1']
		if (datOut1 is not None):
			# print(datOut1.type_desc(), datOut1.as_string())
			# print(datOut1.as_table().row(0))
			# print(datOut1.as_table().row(1))
			# print(datOut1.as_table().col(0))
			# print(datOut1.as_table().cell(2,2))
			# print(datOut1.as_table().as_list())
			pass
			
		datOut2 = comp.out_dats[1]
		if (datOut2 is not None):
			# print(datOut2.type_desc(), datOut2.as_string())
			# print(datOut2.as_table().as_list())
			pass
		
		parNames = comp.par.names()
		if (this.frame == 0):
			for name in parNames:
				print(f"{name}: {comp.par[name].val}")
			
		rgba = comp.par['Rgba']
		rgba.r = .8
		rgba.g = rgba.g + .2
		# comp.par['Rgba'].val = tp.Color(0.1, 0.2, 0.3, .5)

		scale = comp.par['Scale']
		scale.val = 0.0 + this.frame * 0.01
		# print(scale.val)

		translate = comp.par['Translate']
		# translate.val = tp.Float3(11.1, 22.2, 33.3)
		translate.set(tp.Float3(11.1, 22.2, 33.1))
		translate.set([11.1, 22.2, 33.2])
		translate.set(11.1, 22.2, 33.5)


		xyzw = comp.par['Xyzw']
		xyzw.set([1, 2, 3, 4])


		
		if comp.start_next_frame():
			# do work on non-comp members for the next frame here
			pass
		else:
			# we have a problem with TouchEngine starting the next frame.
			# Currently the error will be printed in the console but start_next_frame()
			# will need to be updated to return an enum with the result for error handling.
			pass

		this.frame += 1


	def runComp(self, tox_path):
		comp = tp.Comp(tox_path)
		comp.set_on_frame_callback(self.on_frame, self)

		# comp.start() runs loop or comp.update() to run once
		
		# comp.start(True) # update function starts next frame
		comp.start() # need to manually call start_next_frame() in on_frame callback to start next frame


		# while not (keyboard.is_pressed('q')):
		#	# comp.update(True) # update function starts next frame
		# 	comp.update() #  need to manually call start_next_frame() in on_frame callback to start next frame
		pass


if __name__ == '__main__':
	example = ExampleRunComp()
	example.runComp('TopChopDatIO.tox')
	
	

	

	