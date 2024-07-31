import touchpy as tp
import modules.utils as utils
import numpy as np

class MyComp (tp.Comp):
	def __init__(self):
		super().__init__()
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

		self.frame = 0

		# create a new ChopChannels object 1 sample long with 4 channels
		self.my_chans = tp.ChopChannels(1, channel_names=['a', 'c', 'd'])
		self.my_chans.insert_channel(1, 'b', [1.0])
		self.my_chans.append_channel('e')

	def on_layout_change(self, info):
		print('layout changed:')
		print('out chops:\n', *[f"\t{name}\n" for name in self.out_chops.names])
		print('in chops:\n', *[f"\t{name}\n" for name in self.in_chops.names])

		if 'Openwindow' in self.par.names:
			# only works on supported TD builds
			# official build with support should be available soon
			# self.par['Openwindow'].pulse()
			pass

	def on_frame(self, info):
		if utils.check_key('q'):
			self.stop()
			return

		# all "out" data retrieved from a component (from out TOPs, CHOPs, DATs) must be done 
		# before calling start_next_frame() (pars are an exception)

		# get an out CHOP by index or by name
		out_chop1 = self.out_chops[0]
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
		np_array = out_chop1.as_numpy()
		np_array += 100
			
		if (self.frame == 60):	
			print('\nnp_array.shape:', np_array.shape)
			# print the value of the first sample of each channel
			for i in range(np_array.shape[0]):
				print(f'out_chop1 {out_chop1.chan_names[i]} sample 0 = ', np_array[i][0])

			# note the third channel in the numpy array has 10 added to it, 
			# both the ChopChannels object and the numpy array is a view of the chop data

		# set the first sample of each channel of self.my_chans 
		self.my_chans['a'][0] = np.sin(self.frame * 0.1)
		self.my_chans['b'][0] = np.cos(self.frame * 0.1)

		if 'c' in self.my_chans.names: # will be removed after 600 frames
			self.my_chans['c'][0] = np.tan(self.frame * 0.1)

		self.my_chans['d'][0] = np.arctan(self.frame * 0.1)
		self.my_chans['e'][0] = np.sin(self.frame * 0.1) + 10

		if self.frame == 600:
			# remove the 'c' channel
			self.my_chans.remove_channel('c')

		# set the first in chop with the data from self.my_chans
		self.in_chops[0].from_channels(self.my_chans)

		# set second input chop with numpy array np_array
		self.in_chops[1].from_numpy(np_array)

		# get audio data from 2nd output chop as ChopChannels and as numpy array
		audio_chans = self.out_chops[1].chans()
		audio_array = audio_chans.as_numpy()
		audio_array *= .5 # reduce volume by half
		audio_names = self.out_chops[1].chan_names

		# get time info from the Comp
		time_info = self.time()

		# calculate start and end times in samples
		sample_rate = audio_chans.rate
		samples_per_frame = sample_rate / time_info.rate
		start_time = int(time_info.frame * samples_per_frame)
		end_time = int(start_time + samples_per_frame)

		new_audio_chans = tp.ChopChannels(
			audio_array, 
			rate=sample_rate, 
			is_time_dependent=True, 
			start_time=start_time, 
			end_time=end_time, 
			channel_names=audio_names
		)

		self.in_chops['chopIn3'].from_channels(new_audio_chans)

		self.start_next_frame()

		self.frame += 1



comp = MyComp()
comp.load('TopChopDatIO.tox') 
comp.start()
comp.unload()






