import touchpy as tp
from pprint import pprint

# follow instructions from https://pytorch.org/ to install torch for your system
import torch

import modules.utils as utils
from modules.image_filter import ImageFilter

class MyComp (tp.Comp):
	def __init__(self):
		super().__init__()
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

		# CUDA compatible device is required for this example
		self.device = torch.device('cuda:0') 

		self.frame = 0

		# create an ImageFilter object and move it to the GPU
		self.imag_filter = ImageFilter(
			in_channels=3, 
			out_channels=3, 
			kernel_size=12,
			stride=2,
			groups=3
		).to(self.device)

		# create 1x1080x1920 tensor filled with ones for alpha channel
		# this tensor will be used to create a 4 channel tensor
		self.alpha_tensor = torch.ones(1, 1080, 1920, device=self.device, dtype=torch.float16)

	def on_layout_change(self, info):
		print('layout changed:')
		print('out tops:\n', *[f"\t{name}\n" for name in self.out_tops.names])
		print('in tops:\n', *[f"\t{name}\n" for name in self.in_tops.names])

		# there is a SelectTOP in inside of the component that will select
		# any TOP we specify in the 'Monitortop' parameter
		if 'Monitortop' in self.par.names:
			self.par['Monitortop'].val = 'topIn1'

		if 'Openwindow' in self.par.names:
			# only works on supported TD builds
			# official build with window support should be available soon
			self.par['Openwindow'].pulse()
			pass

		# "RGBA" 8 bit fixed textures in TOPs in TD are actualy stored as BGRA on the GPU.
		# Float types (16 and 32 bit) are stored as RGBA on the GPU.
		# All RG types are stored as RG on the GPU.
		# 
		# The memory layout in all TOPs in TD is interleaved:
		# 		(B, G, R, A, B, G, R, A, ...) for BGRA
		# 		(R, G, B, A, R, G, B, A, ...) for RGBA
		#
		#
		# The fastest way to get data into a CUDA array (tensor etc.) is to use the
		# the same channel order and same memory layout as the TOPs in TD,
		# but most libraries default operations expect a Planar layout: 
		# 		(B, B, B, ..., G, G, G, ..., R, R, R, ..., A, A, A, ...) or
		# 		(R, R, R, ..., G, G, G, ..., B, B, B, ..., A, A, A, ...) or
		# 		(R, R, R, ..., G, G, G, ..., B, B, B, ...) etc.
		#
		# Operations such as tensor.permute((2, 0, 1)).contiguous() can be done in 
		# various libraries to permute the data to the modify the layout but these 
		# have a cost (copying data). Instead we can set flags on the In/Out TOPs in
		# TouchPy to control the layout and channel order of the data when it is copied
		# to/from the texture which must happen in either case. 
		# TouchPy uses the CudaFlags enum to set these flags.
		#
		# The RGBA, BGRA, RGB, BGR flags are used to set the channel order of the data and
		# to specify the number of channels in the data.
		# CHW and HWC flags are used to set the memory layout and the shape of the data, 
		# which follows the torch convention:
		# 		CHW: (C, H, W) channel, height, width - planar layout
		# 		HWC: (H, W, C) height, width, channel - interleaved layout

		# The first output TOP in the TopChopDatIO.tox component is an 8 bit fixed RGBA 
		# texture in TD which is actually stored as BGRA on the GPU.
		# Flags set on OutTops should only be set in the on_layout_change callback
		# or rarely in the on_frame callback (not every frame).
		# Channel order by default is CHW but we can set it to HWC if we want the fastest
		# possible copy to a CUDA array (tensor etc.) and this will now result in BGRA tensor 
		# of the shape (H, W, C) with an interleaved memory layout
		self.out_tops[0].set_cuda_flags(tp.CudaFlags.HWC)

		# The second output TOP in the TopChopDatIO.tox component is a 32 bit float RGBA
		# texture. The channel order is RGBA and the memory layout is interleaved.
		# We can set the flags to RGB CHW to get a tensor of shape (C, H, W) with a planar
		# memory layout. This will result in a tensor with the shape (3, H, W) with the
		# memory layout (R, G, B, R, G, B, ...)
		# We specify the  tp.CudaFlags.CHW flag for clarity but it is not necessary
		# since CHW is default shape.
		self.out_tops[1].set_cuda_flags(tp.CudaFlags.RGB | tp.CudaFlags.CHW)

		self.out_tops[2].set_cuda_flags(tp.CudaFlags.RGB)

	
		if 'Openwindow' in self.par.names:
			# only works on supported TD builds
			# official build with support should be available soon
			# self.par['Openwindow'].pulse()
			pass

	def on_frame(self, info):
		if utils.check_key('q'):
			self.stop()
			return

		if (self.frame == 30):

			# Print the all the gpu data info for each OutTOP
			for name in self.out_tops.names:
				print(f"OutTOP: {name}")

				# note tp.get_dlpack_capsule_info is a utility function that will print the
				# data info for any DLPack compatible object (torch tensor, numpy array etc.)
				pprint(tp.get_dlpack_capsule_info(self.out_tops[name].as_dlpack()), indent=4)
				print()

		# all "out" data retrieved from a component (from out TOPs, CHOPs, DATs) must be done 
		# before calling start_next_frame() (pars are an exception)
		
		# get the data from the first OutTOP as a BGRA HWC tensor (see on_layout_change)
		out_top1 = self.out_tops[0]
		out_top1_tensor = out_top1.as_tensor()

		# this result in a RGB CHW tensor (see on_layout_change)
		out_top2 = self.out_tops[1]

		# get the CudaMemory object for the tensor, this gives access to the raw data
		# for interop with libraries that don't support the DLPack standard such as 
		# CUDA_ARRAY_INTERFACE
		out_top2_cuda_mem = out_top2.cuda_memory()
		if (self.frame == 30):
			print(f"OutTOP3 CudaMemory:")
			print(f"\tsize: {out_top2_cuda_mem.size}")
			print(f"\tptr: {out_top2_cuda_mem.ptr}")
			print(f"\tshape: {out_top2_cuda_mem.desc.shape}")
			print(f"\tstrides: {out_top2_cuda_mem.desc.strides}")
			print(f"\tdata_type: {out_top2_cuda_mem.desc.data_type}")
			print(f"\tcomponent_size: {out_top2_cuda_mem.desc.component_size}")

		# get the DLPack object for the memory, this can be used with any library that
		# supports the DLPack standard
		out_top2_dlpack = out_top2.as_dlpack()

		# get the tensor from the OutTOP3 as a CHW tensor for image filter below
		out_top2_tensor = out_top2.as_tensor()

		# this is an RGB CHW tensor which we'll concat the alpha channel to
		out_top3_tensor = self.out_tops[2].as_tensor()



		self.start_next_frame()

		# convert the tensor to float so we can modify value in the tensor
		# note torch will not automatically normalize the values so the resulting tensor
		# will have values in the range 0.0-255.0
		modified_tensor = out_top1_tensor.float()
		modified_tensor[:, :, :-1] *= .5 # multiply the BGR channels by 0.5
		modified_tensor[:, :, :-1] += 100 # add 100 to the BGR channels
		modified_tensor = torch.clamp(modified_tensor, 0, 255) # clamp the values to 0-255
		modified_tensor = modified_tensor.to(torch.uint8) # convert the tensor back to uint8

		# Set the data back to the OutTOP
		# We could set the flags parameter to inform the InTop of the layout and channel order
		# but it is not necessary since the InTop will implicitly infer the layout and channel order.
		# The InTop is not able to implicitly infer the channel order... (RGB, BGR etc.)
		self.in_tops[0].from_tensor(modified_tensor)

		# Apply the image filter to the OutTOP3 tensor and return a new tensor
		# The filter expects a BCHW tensor so we need to add a batch dimension to the tensor
		# and remove it after the filter is applied
		filtered_tensor = self.imag_filter(out_top2_tensor.unsqueeze(0)).squeeze(0) 

		# Set the data back to the OutTOP3
		self.in_tops[1].from_tensor(filtered_tensor)

		# concat the alpha channel to the tensor
		rgba_tensor = torch.cat((out_top3_tensor, self.alpha_tensor), dim=0)

		# Set the data back to the OutTOP3
		self.in_tops[2].from_tensor(rgba_tensor)

		self.frame += 1



comp = MyComp()
comp.load('TopChopDatIO.tox') 
comp.start()
comp.unload()






