import keyboard # optional, used to quit the loop
import numpy as np
import torch
from pathlib import Path
import sys
from PIL import Image
import cv2 as cv

localImportPath = Path(__file__).parents[3] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

torch.cuda.set_device(0)

from diffusers import StableDiffusionXLAdapterPipeline, AutoPipelineForImage2Image, T2IAdapter, EulerAncestralDiscreteScheduler, AutoencoderKL
from controlnet_aux.canny import CannyDetector
from diffusers.utils import load_image, make_image_grid
import torch


class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0

		# load adapter
		adapter = T2IAdapter.from_pretrained("TencentARC/t2i-adapter-canny-sdxl-1.0", torch_dtype=torch.float16, varient="fp16").to("cuda")
		vae=AutoencoderKL.from_pretrained("madebyollin/sdxl-vae-fp16-fix", torch_dtype=torch.float16)
		
		self.pipeline = StableDiffusionXLAdapterPipeline.from_pretrained("stabilityai/sdxl-turbo", vae=vae, adapter=adapter, torch_dtype=torch.float16, variant="fp16")
		self.pipeline = self.pipeline.to("cuda")
		#self.pipeline.enable_xformers_memory_efficient_attention()
		self.canny_detector = CannyDetector()
		self.inputBuffer = None
		self.outBuffer = None

		
		self.testImage = load_image("./circle.png")

	@staticmethod
	def on_layout_change(comp, info):
		#comp.out_tops[0].set_cuda_flags(tp.CudaFlags.BGR | tp.CudaFlags.HWC)
		pass

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		
		#read Out TOP from the tox we loaded
		this.inputBuffer = comp.out_tops[0].as_tensor()

		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	
		
		#inference
		
		

		okArray = np.array(this.testImage)
		#print("+++++++ OK ++++")
		#print(ok.shape, ok.dtype, ok.strides, ok.flags['C_CONTIGUOUS'])
		#print("+++++++ OK ++++")

		#cpuBuffer= this.inputBuffer.cpu().numpy().astype(np.uint8) * 255
		#cv.imshow('Example - Show image in window', cpuBuffer.transpose(1, 2, 0))
		
		canny = this.canny_detector(okArray, detect_resolution=384, image_resolution=1024, output_type="pil")
		
		canny.show()

		generator = torch.manual_seed(0)
		


		control_image = this.testImage
		prompt = "pale golden rod circle with old lace background"


		this.outBuffer = this.pipeline(prompt, control_image, generator=generator, num_inference_steps=2,output_type="pt")
		#torch.cuda.synchronize()
		#print(this.outBuffer, this.outBuffer.device, this.outBuffer.shape)

		#canny = np.array(canny)
			#print(canny, canny.device)
		#this.outBuffer = torch.from_numpy(canny/255).cuda()
		comp.in_tops[0].from_tensor(this.outBuffer)
		
		#	output = np.asarray(image)
		#this.outBuffer = torch.from_numpy(image).cuda()
		#comp.in_tops[0].from_tensor(this.outBuffer)
		#comp.in_tops[0].from_tensor(image)
		
		this.frame += 1

	def runComp(self, tox_path):
		comp = tp.Comp(tox_path, flags=tp.CompFlags.INTERNAL_TIME_AUTO)
		comp.set_on_layout_change_callback(self.on_layout_change, self)
		comp.set_on_frame_callback(self.on_frame, self)
		comp.start()
		comp.unload()
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('tox/yolo.tox')

