import sys
import os
import keyboard
import numpy as np
import torch
from PIL import Image

# get the path to touchpy.pyd: ../out/build/x64-release
path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-release'))
# path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-relwithdebuginfo'))
sys.path.append(path)

import touchpy as tp



class ExampleRunComp:
	def __init__(self):
		self.frame = 0
		self.a = 1
		self.b = 2
		self.test_array = np.array([[1],[2],[3],[4],[5],[6],[7],[8],[9],[10]], dtype=np.float32)
		self.test_array_chan_names = [f"channel{i}" for i in range(10)]

	def test(self, chans):
		print(chans)
		pass

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q')):
			comp.stop()
			return
		
		# prev_cuda_ptr = 0
		cudamem = comp.out_tops[0].cuda_memory()
		comp.in_tops[0].copy_cuda_memory(cudamem)
		
		# prev_cuda_ptr = 0	
		# tensor = comp.out_tops[0].as_tensor()
		# cuda_ptr = tensor.data_ptr()
		# if cuda_ptr != 0 and cuda_ptr != prev_cuda_ptr:
		# 	tensor_size = tensor.element_size() * tensor.numel()
		# 	print("tensor: ", tensor.data_ptr(), tensor_size, tensor.shape, tensor.dtype)
		# 	prev_cuda_ptr = cuda_ptr
		# 	tensor_copy = tensor.clone()

		# if (this.frame == 4):
		# 	cudamem = comp.out_tops[0].cudaMemory()
		# 	print("cudamem: ",  cudamem.ptr, cudamem.ptr.data(), cudamem.size)
		# 	tensor = comp.out_tops[0].as_tensor()
		# 	print(tensor.shape, tensor.dtype)
		# 	image = Image.fromarray(tensor.cpu().numpy().astype('uint8'), 'RGBA')
		# 	image.show()
		# 	comp.in_tops[0].from_tensor(tensor)

		comp.in_chops[0].from_numpy(this.test_array)
		this.test_array += 1

		# get a reference to the numpy array, some functions that do not copy 
		# will not work with this passed as an argument, such as ChopLink.from_numpy() 
		# faster than a copy though for large arrays... good for read only operations
		chans1_ref = comp.out_chops['chopOut1'].as_numpy_ref() 
		# comp.in_chops[1].from_numpy(chans1) # this will not work!

		chans2 = comp.out_chops[1].as_numpy()
		chans2_names = comp.out_chops[1].chan_names()
		# print(chans2_names)

		comp.in_chops[1].from_numpy(chans2) # this will work!
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
			
		# rgba = comp.par['Rgba'].val
		# rgba.r = .1
		# rgba.g = .2
		# rgba.b = .3
		comp.par['Rgba'].val.a = .5 + this.frame * 0.01
		# comp.par['Rgba'].val = tp.Color(0.1, 0.2, 0.3)

		scale = comp.par['Scale']
		scale.val = 0.0 + this.frame * 0.01
		# print(scale.val)




		this.frame += 1

	def runComp(self, tox_path):
		comp = tp.Comp(tox_path)
		comp.set_on_frame_callback(self.on_frame, self)
		comp.start()

		# datTable = tp.DatTable()
		# datTable.from_list([['a', 'b', 'c'], [1, 2.3, 3], ['g', 'h', 'i']], True)
		# print(datTable.as_list())

		# del(comp)
		# while not (keyboard.is_pressed('q')):
		# 	comp.update()
		pass


if __name__ == '__main__':
	example = ExampleRunComp()
	tox_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'TopChopDatIO.tox'))
	example.runComp(tox_path)



	
	

	

	