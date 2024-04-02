import keyboard # optional, used to quit the loop
import numpy as np
import torch
from pathlib import Path
import sys
from PIL import Image


localImportPath = Path(__file__).parents[3] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

torch.cuda.set_device(0)

from diffusers import AutoPipelineForImage2Image
from diffusers.utils import load_image
import torch


class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0
		self.pipeline = AutoPipelineForImage2Image.from_pretrained("stabilityai/sdxl-turbo", torch_dtype=torch.float16, variant="fp16")
		self.pipeline = self.pipeline.to("cuda")
		self.init_image = load_image("https://huggingface.co/datasets/huggingface/documentation-images/resolve/main/diffusers/cat.png")
		self.init_image = self.init_image.resize((512, 512))
		self.inputBuffer = None
		self.outBuffer = None

	@staticmethod
	def on_layout_change(comp, info):
		comp.out_tops[0].set_cuda_flags(tp.CudaFlags.RGB)

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
		prompt = "wizard, gandalf, lord of the rings, detailed, fantasy, cute, adorable, Pixar, Disney, 8k"

		image = this.pipeline(prompt, image=this.inputBuffer, strength=0.6, guidance_scale=0.0, num_inference_steps=2).images[0]
		output = np.asarray(image)
		this.outBuffer = torch.from_numpy(output).cuda()
		comp.in_tops[0].from_tensor(this.outBuffer)
		
		
		

		
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

