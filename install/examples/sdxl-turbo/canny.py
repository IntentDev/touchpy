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

from diffusers import StableDiffusionXLAdapterPipeline, T2IAdapter, EulerAncestralDiscreteScheduler, AutoencoderKL
from diffusers.utils import load_image, make_image_grid
import torch


class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0

		# load adapter
		adapter = T2IAdapter.from_pretrained("Adapter/t2iadapter", subfolder="sketch_sdxl_1.0", torch_dtype=torch.float16, adapter_type="full_adapter_xl").to("cuda")
		vae=AutoencoderKL.from_pretrained("madebyollin/sdxl-vae-fp16-fix", torch_dtype=torch.float16)
		
		self.pipe = StableDiffusionXLAdapterPipeline.from_pretrained("stabilityai/sdxl-turbo", vae=vae, adapter=adapter, torch_dtype=torch.float16, variant="fp16")
		self.pipe = self.pipe.to("cuda")
		
		self.inputBuffer = None
		self.outBuffer = None
		self.testImage = load_image("./idzard.jpg")

	@staticmethod
	def on_layout_change(comp, info):
		comp.out_tops[1].set_cuda_flags(tp.CudaFlags.BGR)
		
		pass

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		
		#read Out TOPs from the tox we loaded
		webcam = comp.out_tops[0].as_tensor()

		
		canny = comp.out_tops[1].as_tensor()
		print(canny.shape)
		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	
		
		#canny = canny.cpu().numpy()	
		#canny = cv.cvtColor(canny, cv.COLOR_BGR2GRAY)
		print(canny.shape)


		prompt = "Mystical fairy in real, magic, 4k picture, high quality"
		negative_prompt = "extra digit, fewer digits, cropped, worst quality, low quality, glitch, deformed, mutated, ugly, disfigured"
		# fix the random seed, so you will get the same result as the example
		generator = torch.Generator().manual_seed(42)

		result = this.pipe(
			prompt=prompt,
			negative_prompt=negative_prompt,
			image=canny,
			width=512,
			height=512, #output_type="pt"
			generator=generator,
    		guidance_scale=7.5
		).images[0]	

		
		result.show()
		#print(result)
		#out = np.array(result)
		#cv.imshow("test", out)
		
		#print(canny, canny.device)
		#out = torch.from_numpy(out/255).cuda()
		#comp.in_tops[0].from_tensor(out)


		this.frame += 1
		#print("Frame: ", this.frame)

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

