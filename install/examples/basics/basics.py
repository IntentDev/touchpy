import keyboard # optional, used to quit the loop
import numpy as np
import torch

import touchpy as tp

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# change the logging level and enable/disable logging to console/file
# currently defaults to LogLevel.INFO, console=True, file=False
#
# tp.init_logging(level=tp.LogLevel.WARNING, console=True, file=True)

class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop

		# used to test InChopLink
		self.frame = 0
		self.test_array = np.array([[1],[2],[3],[4],[5],[6],[7],[8],[9],[10]], dtype=np.float32)
		self.test_array_chan_names = [f"chn{i}" for i in range(10)]

	# called when the layout of the comp changes which means:
	# 	- any out, in, or parameter was added, removed or renamed
	# 	- called at least once when the comp is loaded
	@staticmethod
	def on_layout_change(comp, this):
		print('layout changed:')
		print('in tops:', comp.in_tops.names)
		print('out tops:', comp.out_tops.names)
		print('in chops:', comp.in_chops.names)
		print('out chops:', comp.out_chops.names)
		print('in dats:', comp.in_dats.names)
		print('out dats:', comp.out_dats.names)
		print('pars:', comp.par.names)

		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# set some flags to specify the shape a tops texture should have 
		# when converted to a tensor or array 
		# default is R, RG, RGBA / CHW for 32-bit and 16 bit textures
		# R, RG, BGRA / CHW for 8-bit textures
		# swizzling of 4 to 3 channels is supported
		# swapping of BGR to RGB, BGRA to RGBA and vice versa is supported
		#
		# comp.out_tops[0].set_cuda_flags(tp.CudaFlags.RGBA | tp.CudaFlags.HWC)
		# comp.out_tops['topOut1'].set_cuda_flags(tp.CudaFlags.RGB)

		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# not yet supported in public releases of TouchDesigner
		#
		# if 'Openwindow' in comp.par.names:
		# 	comp.par['Openwindow'].pulse() # pulse the Openwindow parameter
		return

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q') and keyboard.is_pressed('ctrl')):
			comp.stop() # stop running the comp
			return
		
		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# get the texture data of the first Out TOP as a tensor by index or name
		#
		tensor1 = comp.out_tops[0].as_tensor()
		# tensor = comp.out_tops['topOut1'].as_tensor()

		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# get the first chop as a numpy array and its channel names
		#
		chans0 = comp.out_chops[0].as_numpy()
		names0 = comp.out_chops[0].chan_names

		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# get the first dat as a table
		#
		table = comp.out_dats[0].as_table()

		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# do work after calling next frame for fastest performance with the
		# cost of a frame of latency, otherise work before
		# all reading of data from out operators should be done before calling next frame
		# (pars can be read and written at any time)
		#
		comp.start_next_frame()

		# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		# simply copy the first input top to the first output top
		#
		comp.in_tops['topIn1'].from_tensor(tensor1) # set the first top from a tensor
		
		# comp.in_tops[0].from_tensor(tensor, flags=tp.CudaFlags.RGB)

		comp.in_chops[0].from_numpy(chans0, names0)
		# comp.in_chops[0].from_numpy(this.test_array, this.test_array_chan_names)

		comp.in_dats[0].from_table(table)

		# pars: ['Momentary', 'Scale', 'Openwindow', 'Float', 'Xyzw', 
		# 'Translate', 'Rotate', 'Monitortop', 'File', 'Pulse', 'Int', 'Menu', 
		# 'Toggle', 'Rgba', 'Uvw']

		comp.par['Scale'].val = 2.

		Xyzw = comp.par['Xyzw']
		Xyzw.set(1., 2., 3., 4. ) 
		Xyzw.x = 5.
		# my_point = tp.Float4(0., 0., 0., 0.)
		# Xyzw.set(my_point)

		this.frame += 1

	def runComp(self, tox_path):
		# create a comp object and specify a path to a tox file
		comp = tp.Comp(tox_path)
		# comp = tp.Comp(tox_path, flags=tp.CompFlags.INTERNAL_TIME_AUTO)

		comp.set_on_layout_change_callback(self.on_layout_change, self)
		comp.set_on_frame_callback(self.on_frame, self)

		comp.start() # start the comp, blocks with CompFlags.InternalTimeAuto and CompFlags.InternalTimeSemiAuto

		comp.unload() # should be called to properly unload the comp (especially if Python exits immediately after this)
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('TopChopDatIO.tox')

