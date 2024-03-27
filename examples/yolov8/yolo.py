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
		self.inputBuffer = None
		self.outBuffer = None

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		
		#read Out TOP from the tox we loaded
		this.inputBuffer = comp.out_tops[0].as_tensor()

		# convert RGBA tensor to a RGB tensor
		if (this.inputBuffer.shape[0] == 4):
			this.inputBuffer = this.inputBuffer[:3]	

		# convert RGB tensor to BGR and flip it upside down (as Yolo is expecting OpenCV format)
		this.inputBuffer = torch.flip(this.inputBuffer, [0,1])
			
		#inference
		results = this.model(this.inputBuffer.unsqueeze(0),stream=True, device=0)
		result = next(results)

		#write pose points to input CHOP
		keypoints = result.keypoints.data.cpu().numpy()
		keypoints = keypoints.astype(np.float32)
		#reshape keypoints to 3 channels with 17 samples]
		keypoints = keypoints.reshape(3,-1)

		comp.in_chops[0].from_numpy(keypoints)

		fps = 1000 / ( result.speed["preprocess"]+result.speed["inference"]+result.speed["postprocess"])
		print(f"{fps} maxfps")
		
		#plot opencv annotations in a numpy array
		annotatedArray = result.plot()
			
		#convert from BGR to RGB and flip vertically (to match TouchDesigner format)
		annotatedArray = cv2.cvtColor(annotatedArray, cv2.COLOR_BGR2RGB)

		#copy to GPU
		this.outBuffer = torch.from_numpy(annotatedArray).float().cuda()
			
		# convert tensor from HWC to CHW
		this.outBuffer = torch.permute(this.outBuffer, (2,0,1))

		#convert tensor color from 0-255 to 0-1 range
		this.outBuffer = this.outBuffer / 255.0
		this.outBuffer = torch.flip(this.outBuffer, [1])	
				
		######### Copy on next frame (Fast) ###################
		comp.start_next_frame()	
		
		comp.in_tops[0].from_tensor(this.outBuffer)
		comp.in_chops[0].from_numpy(keypoints)
		this.frame += 1

	def runComp(self, tox_path):
		comp = tp.Comp(tox_path, run_mode=tp.RunMode.InternalTimeSemiAuto)
		comp.set_on_frame_callback(self.on_frame, self)
		comp.start()
		comp.unload()
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('tox/yolo.tox')

