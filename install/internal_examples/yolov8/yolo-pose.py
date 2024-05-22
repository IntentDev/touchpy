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
		#self.model = YOLO("yolov8x.pt")
		self.model = YOLO("G:\Shared drives\Projects\TouchPy\yolov8\models\yolov8x-pose.engine", verbose=False)
		self.inputBuffer = None
		self.outBuffer = None

	@staticmethod
	def on_layout_change(comp, info):
		comp.out_tops[0].set_cuda_flags(tp.CudaFlags.BGR)

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
		results = this.model(this.inputBuffer.unsqueeze(0),stream=True, device=0)
		result = next(results)

		#write pose points to input CHOP
		keypoints = result.keypoints.data.cpu().numpy()
		keypoints = keypoints.astype(np.float32)
		#reshape keypoints to 3 channels with 17 samples]
		keypoints = keypoints.reshape(3,-1)

		comp.in_chops[0].from_numpy(keypoints)

		fps = 1000 / ( result.speed["preprocess"]+result.speed["inference"]+result.speed["postprocess"])
		#print(f"{fps} maxfps")
		
		#plot opencv annotations in a numpy array
		annotatedArray = result.plot(boxes=False, labels=False)
			
		this.outBuffer = torch.from_numpy(annotatedArray).cuda()
		comp.in_tops[0].from_tensor(this.outBuffer, flags=tp.CudaFlags.BGR)
		comp.in_chops[0].from_numpy(keypoints)
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

