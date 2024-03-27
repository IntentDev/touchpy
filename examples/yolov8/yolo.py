import keyboard # optional, used to quit the loop
import numpy as np
import torch
from pathlib import Path
import sys

from ultralytics import YOLO
localImportPath = Path(__file__).parents[2] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0
		self.model = YOLO("yolov8m-pose.pt")


	@staticmethod
	def on_frame(comp, this):

		# optional used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		if (this.frame == 1):
			print("tox started")
		tensor = comp.out_tops[2].as_tensor()
		if (this.frame == 2):
				print("tensor shape: ", tensor.shape, "tensor dtype: ", tensor.dtype, 
				"tensor device: ", tensor.device, "tensor layout: ", tensor.layout, 
				"tensor strides: ", tensor.stride(), "tensor is_contiguous: ", tensor.is_contiguous())
		
		results = this.model(tensor.unsqueeze(0), show=True, conf=0.3)
		comp.in_tops[0].from_tensor(tensor)
		print(this.frame)		
		this.frame += 1

	def runComp(self, tox_path):
		# create a comp object and specify a path to a tox file
		
		comp = tp.Comp(tox_path)

		comp.set_on_frame_callback(self.on_frame, self)

		comp.start() # start the comp, blocks with RunMode.InternalTimeAuto and RunMode.InternalTimeSemiAuto
		comp.unload() # should be called to properly unload the comp (especially if Python exits immediately after this)
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('TopChopDatIO.tox')

