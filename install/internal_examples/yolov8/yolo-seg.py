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
		#self.model = YOLO("yolov8x-seg.pt")
		self.model = YOLO("G:\Shared drives\Projects\TouchPy\yolov8\models\yolov8x-seg.engine", verbose=False)
		self.inputBuffer = None
		self.outBuffer = torch.zeros((640, 640,	), dtype=torch.float32, device='cuda')

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
		resultsgen = this.model(this.inputBuffer.unsqueeze(0),stream=True, device=0)
		results = next(resultsgen)
		if results.masks is not None:
			# print(results.masks.data.shape, results.masks.data.device)

			# names = results.names
			# print(names)
			boxes = results.boxes
			# print(boxes)
			
			this.outBuffer.fill_(0)
			for i, mask_data in enumerate(results.masks.data):
				cls_id = int(results.boxes.cls[i].cpu())
				print(cls_id)
				val = (cls_id + 1) / 99
				# val = (i + 1) / (results.masks.data.shape[0] + 1)
				mask_data = mask_data * val
				this.outBuffer += mask_data
				# cls_ = results.names[int(results.boxes.cls[0].cpu())]

			comp.in_tops[0].from_tensor(this.outBuffer.unsqueeze(0))
			
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

