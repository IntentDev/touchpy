import keyboard # optional, used to quit the loop
import numpy as np
import torch

from image_filter import ImageFilter

import touchpy as tp

# # interface class to pass a cuda stream pointer to torch
# class CudaStream:
# 	def __init__(self, stream, device=torch.device('cuda'), device_index=0):
# 		self.stream_id = stream
# 		self.device = device
# 		self.device_index = device_index
# 		self.device_type = 'cuda'

# 	@property
# 	def cuda_stream(self):
# 		return self.stream_id
	
# 	@cuda_stream.setter
# 	def cuda_stream(self, stream):
# 		self.stream_id = stream


class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.device = torch.device('cuda')
		self.imag_filter = ImageFilter().to(self.device)

		# used to test InChopLink
		self.frame = 0
		self.test_array = np.array([[1],[2],[3],[4],[5],[6],[7],[8],[9],[10]], dtype=np.float32)
		self.test_array_chan_names = [f"chn{i}" for i in range(10)]

		self.stream = None
	
	@staticmethod
	def on_layout_change(comp, this):
		print('layout changed:')
		print('in tops:', comp.in_tops.count, comp.in_tops.names)
		print('out tops:', comp.out_tops.count, comp.out_tops.names)
		print('in chops:', comp.in_chops.count, comp.in_chops.names)
		print('out chops:', comp.out_chops.count, comp.out_chops.names)
		print('in dats:', comp.in_dats.count, comp.in_dats.names)
		print('out dats:', comp.out_dats.count, comp.out_dats.names)
		print('pars:', comp.par.count, comp.par.names)
		# comp.out_tops[1].set_cuda_flags(tp.CudaFlags.BGRA | tp.CudaFlags.HWC)
		comp.out_tops[1].set_cuda_flags(tp.CudaFlags.RGB)

		this.stream = torch.cuda.ExternalStream(comp.cuda_stream(), device=this.device)
		# this.stream = CudaStream(comp.cuda_stream())
		# this.stream = torch.cuda.Stream().cuda_stream

		# print("cuda stream:", cuda_stream, ", type: ", type(cuda_stream))
	
		# print("cuda stream:", torch_stream, ", type: ", type(torch_stream))
		# comp.out_tops[0].set_cuda_stream(this.stream)
		# comp.out_tops[0].set_cuda_stream(this.torch_stream.cuda_stream)



	@staticmethod
	def on_frame(comp, this):

		# optional used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return

		time_info = comp.time()
		# print('time_info:', time_info)
		# copy out_chop to in_chop with channel names. 
		arr = comp.out_chops[0].as_numpy()
		names = comp.out_chops[0].chan_names
		# arr *= 2
		# comp.in_chops[0].from_numpy(arr, names)
		
		# set in_chops[0] with local data
		# comp.in_chops[0].from_numpy(this.test_array, this.test_array_chan_names)

		temp = comp.out_chops[0].chans()
		# audioChannels = comp.out_chops[0].chans()
		# print("audio channels - num_channels:", audioChannels.num_chans, "numSamples:", audioChannels.num_samples, "is_time_dependent:", audioChannels.is_time_dependent, "sample_rate:", audioChannels.rate, "start_time:", audioChannels.start_time, "end_time:", audioChannels.end_time)
  
		# audioChannels = tp.ChopChannels(arr, temp.rate, temp.is_time_dependent, temp.start_time, temp.end_time, names)
  
		samples_per_frame = temp.rate / time_info.rate
		start_time = int(time_info.frame * samples_per_frame)
		end_time = int(start_time + samples_per_frame)

		# print("sameples_per_frame:", samples_per_frame, "start_time:", start_time, "end_time:", end_time)
		audioChannels = tp.ChopChannels(arr, temp.rate, temp.is_time_dependent, start_time, end_time, names)


		# audioChannels = tp.ChopChannels()
		# audioChannels.from_numpy(arr, temp.rate, temp.is_time_dependent, temp.start_time, temp.end_time, names)

		# comp.in_chops[0].from_numpy(audioChannels.as_numpy(), audioChannels.chan_names)
		# if this.frame > 100:
		# 	comp.in_chops[0].from_channels(audioChannels)

		empty_chans = tp.ChopChannels(10, channel_names=['a', 'b', 'c', 'd'])


		# print(empty_chans)
		# empty_chans.is_time_dependent = True
		# empty_chans.rate = 60
		# empty_chans.start_time = int(time_info.frame * 10)
		# empty_chans.end_time = int(empty_chans.start_time + 10)
		# print(empty_chans)
  
		# empty_chans.set_values(0, [1, 2, 1, 2, 3, 0, 1, 2, 1, 1])
		# empty_chans.set_values('b', [4, 2, 4, 2, 3, 0, 1, 2, 1, 1])
		# empty_chans.set_values('c', [0, 1, 2, 1, 1])
		# empty_chans.set_values('d', [0, 1, 2, 1, 1], 4)
  
		# empty_chans.set_value(0, 1, 4)
		# empty_chans.set_value('c', 4, 2)
		# empty_chans[0][0] = 3 
		# print(empty_chans[0])
		# empty_chans['c'][0] = 5 
		# print(empty_chans['c'])

		empty_chans.append_channel('e', [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
		empty_chans.append_channel()
		empty_chans.remove_channel('b')
		# empty_chans.remove_channel(2)
		empty_chans.insert_channel(3, 'f', [1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

		# print(empty_chans.chan_names)

		comp.in_chops[1].from_channels(empty_chans)

		# update the local data
		this.test_array += .01

		# chans2 = comp.out_chops[1].as_numpy()
		# chans2_names = comp.out_chops[1].chan_names
		# # print(chans2_names)

		# # retrieve by name and set as local variable
		# in_chop2 = comp.in_chops['chopIn2']
		# in_chop2.from_numpy(chans2, chans2_names)

		# # print some channel data
		chans3 = comp.out_chops[2]
		arr = chans3.as_numpy()
		# print(arr)
		comp.in_chops[2].from_numpy(arr, chans3.chan_names)

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
		
		# get the parameter named Rgba and set it's values in different ways
		# setting individual components is not ideal, but it's possible
		# better set .val to a new tp.Color object or use .set()
		# rgba = comp.par['Rgba']
		# rgba.r = .8
		# rgba.g = rgba.g + .2
		# comp.par['Rgba'].val = tp.Color(0.1, 0.2, 0.3, .5)

		# get the parameter named Scale and set it's value
		scale = comp.par['Scale']
		# scale.val = 1.0 #+ this.frame * 0.01
		# print(scale.val)

		# get the parameter named Translate and set it's value
		# .set() can take a list, tuple, individual values or the correct type for the parameter
		translate = comp.par['Translate']
		# translate.val = tp.Float2(11.1, 22.2)
		# translate.set(tp.Float2(11.1, 22.2))
		# translate.set([11.1, 22.2])
		# translate.set(11.1, 22.5)


		# copy the cuda memory from out_top_link to in_top_link
		cudamem = comp.out_tops[0].cuda_memory(sync_cuda_stream=True)
		comp.in_tops[0].copy_cuda_memory(cudamem)

		# copy the cuda memory from out_top_link to in_top_link
		# cudamem = comp.out_tops[1].cuda_memory()
		# comp.in_tops[1].copy_cuda_memory(cudamem)

		with torch.cuda.stream(this.stream):	
			with torch.no_grad():
				# tensor = comp.out_tops[0].as_tensor() # get the first top as a tensor
				# tensor2 = tensor * 2 # do some work on the tensor
				# comp.in_tops[0].from_tensor(tensor2)

				tensor = comp.out_tops[1].as_tensor(sync_cuda_stream=True)
				if (this.frame == 2):
					
					print("tensor shape: ", tensor.shape, "tensor dtype: ", tensor.
					dtype, "tensor device: ", tensor.device, "tensor layout: ", tensor.layout, 
					"tensor strides: ", tensor.stride(), "tensor is_contiguous: ", tensor.is_contiguous())

				# filter tensor only works with 32bit float data (comp.out_tops[2] is 32bit float in this example)
				# filter expects (b, c, h, w) layout
				# tensor2 = this.imag_filter(tensor.unsqueeze(0)).squeeze(0) 
				tensor2 = tensor.clone()

				if (this.frame == 2):
					print("tensor2 shape: ", tensor2.shape, "tensor2 dtype: ", tensor2.dtype, 
					"tensor2 device: ", tensor2.device, "tensor2 layout: ", tensor2.layout, 
					"tensor2 strides: ", tensor2.stride(), "tensor2 is_contiguous: ", tensor2.is_contiguous())
				
				# comp.in_tops[2].from_tensor(tensor2, this.stream)
				comp.in_tops[1].from_tensor(tensor2, flags=tp.CudaFlags.RGB)
				# comp.in_tops[2].from_tensor(tensor2)
				pass

			comp.start_next_frame()
		
			this.frame += 1

	def runComp(self, tox_path):
		# create a comp object and specify a path to a tox file
		# comp = tp.Comp(tox_path)
		# comp = tp.Comp(tox_path, flags=tp.CompFlags.INTERNAL_TIME_AUTO)
		comp = tp.Comp(tox_path, flags=tp.CompFlags.INTERNAL_TIME_AUTO | tp.CompFlags.CUDA_STREAM_INTERNAL)

		comp.set_on_layout_change_callback(self.on_layout_change, self)
		comp.set_on_frame_callback(self.on_frame, self)

		comp.start() # start the comp, blocks with CompFlags.InternalTimeAuto and CompFlags.InternalTimeSemiAuto

		comp.unload() # should be called to properly unload the comp (especially if Python exits immediately after this)
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('TopChopDatIO.tox')

