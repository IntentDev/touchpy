import touchpy as tp
import utils
import numpy as np

class MyComp (tp.Comp):
	def __init__(self):
		super().__init__()
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

		self.frame = 0

		# create a new ChopChannels object 1 sample long with 4 channels
		self.my_chans = tp.ChopChannels(1, channel_names=['a', 'c', 'd'])
		self.my_chans.append_channel('e', [0.0])
		self.my_chans.insert_channel(1, 'b', [0.0])

	
	def on_layout_change(self, info):
		print('layout changed:')
		print('in chops:\n', *[f"\t{name}\n" for name in self.in_chops.names])
		print('out chops:\n', *[f"\t{name}\n" for name in self.out_chops.names])

		if 'Openwindow' in self.par.names:
			# only works on supported TD builds
			# official build with support should be available soon
			self.par['Openwindow'].pulse()

	def on_frame(self, info):
		if utils.check_key('q'):
			self.stop()
			return

		# all "out" data retrieved from a component (from out TOPs, CHOPs, DATs) must be done 
		# before calling start_next_frame() (pars are an exception)
		# it's typically a good idea to call start_next_frame() before doing any processing
		# and setting of data but it's not required (faster, with potential of 1 frame of latency)

		# get the first out CHOP by index
		out_chop1 = self.out_chops[0]

		# get the first out CHOP by name
		out_chop1 = self.out_chops['chopOut1']

		if (self.frame == 60):
			print('out_chop1 channel names:', out_chop1.chan_names)

			# get a ChopChannels object from the chop
			out_chop1_chans = out_chop1.chans()

			print('out_chop1_chans:')
			print('\tnum channels:', out_chop1_chans.num_chans)
			print('\tnum samples:', out_chop1_chans.num_samples)
			print('\tsample rate', out_chop1_chans.rate)
			print('\tis time dependent:', out_chop1_chans.is_time_dependent)
			print('\tstart time:', out_chop1_chans.start_time)
			print('\tend time:', out_chop1_chans.end_time)

			# get the first sample of the first channel by index
			print('\tout_chop1_chans[0][0] = ', out_chop1_chans[0][0])

			# get the first sample of the third channel by name
			print('\tout_chop1_chans["test_chan3"][0] = ', out_chop1_chans['test_chan3'][0])

			# add 10 to the first sample of the third channel
			out_chop1_chans['test_chan3'][0] += 10
			print('\tout_chop1_chans["test_chan3"][0] = ', out_chop1_chans['test_chan3'][0])

	
			# get all the values of the chop as a numpy array
			out_chop1_arr = out_chop1.as_numpy()
			out_chop1_arr += 100
			
			print('\nout_chop1_arr.shape:', out_chop1_arr.shape)
			# print the value of the first sample of each channel
			for i in range(out_chop1_arr.shape[0]):
				print(f'out_chop1 {out_chop1.chan_names[i]} sample 0 = ', out_chop1_arr[i][0])

			# note the third channel in the numpy array has 10 added to it, 
			# both the ChopChannels object and the numpy array is a view of the chop data

		# set the first sample of each channel of self.my_chans 
		self.my_chans['a'][0] = np.sin(self.frame * 0.1)
		self.my_chans['b'][0] = np.cos(self.frame * 0.1)
		self.my_chans['c'][0] = np.tan(self.frame * 0.1)
		self.my_chans['d'][0] = np.arctan(self.frame * 0.1)
		# self.my_chans['e'][0] = self.my_chans['a'][0] + self.my_chans['b'][0]

		# if (self.frame == 600):
		# 	self.my_chans.remove_channel('c')

		# set the first in chop with the data from self.my_chans
		self.in_chops[0].from_channels(self.my_chans)




		self.start_next_frame()


		self.frame += 1



comp = MyComp()
comp.load('TopChopDatIO.tox') 
comp.start()
comp.unload()






