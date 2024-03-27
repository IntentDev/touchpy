import keyboard # optional, used to quit the loop
import numpy as np
import torch
from pathlib import Path
import sys
import cv2

from ultralytics import YOLO

localImportPath = Path(__file__).parents[2] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

torch.cuda.set_device(0)

class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0
		self.model = YOLO("models/yolov8s-pose.pt")
		


	@staticmethod
	def on_frame(comp, this):

		# optional used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		if (this.frame == 1):
			print("tox started")
		
		tensor = comp.out_tops[0].as_tensor()

		# convert an RGBA tensor to a RGB tensor
		if (tensor.shape[0] == 4):
			tensor = tensor[:3]	

		# convert RGB tensor to BGR and flip it upside down (as it is expecting OpenCV format)
		tensor = torch.flip(tensor, [0,1])
			
			
		results = this.model(tensor.unsqueeze(0), show=True, stream=True, device=0)
		for result in results:
			
			#print(result.keypoints, result.keypoints.type())
			
			
			annotatedArray = result.plot().images
			
			#convert from BGR to RGB and flip vertically (to match TouchDesigner format)
			annotatedArray = cv2.cvtColor(annotatedArray, cv2.COLOR_BGR2RGB)[...,::-1,:]
			
			#copy to GPU
			tensor = torch.from_numpy(annotatedArray).float().cuda()
			
			# convert tensor from HWC to CHW
			out = torch.permute(tensor, (2,0,1))

			#convert tensor color from 0-255 to 0-1 range
			out = out / 255.0
			
			comp.in_tops[0].from_tensor(out)
			
				
		this.frame += 1

	def runComp(self, tox_path):
		comp = tp.Comp(tox_path)

		comp.set_on_frame_callback(self.on_frame, self)

		comp.start()
		comp.unload()
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('tox/yolo.tox')

