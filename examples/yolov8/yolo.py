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

torch.cuda.set_device(0)

class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0
		self.model = YOLO("models/yolov8m-pose.pt")
		


	@staticmethod
	def on_frame(comp, this):

		# optional used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		if (this.frame == 1):
			print("tox started")
		tensor = comp.out_tops[0].as_tensor()
		
		tensor.half()

		# convert an rgba tensor to a rgb tensor
		if (tensor.shape[0] == 4):
			tensor = tensor[:3]	



		# flip a tensor image upside down
		tensor = torch.flip(tensor, [1])
			
		
		
		results = this.model(tensor.unsqueeze(0), show=True, stream=True, device=0)
		for result in results:
			print(result.keypoints)
			#annotatedArray = r.plot()
			#comp.in_tops[0].from_tensor(torch.from_numpy(annotatedArray))
			#comp.in_tops[0].from_tensor(tensorOrg)
				
		this.frame += 1

	def runComp(self, tox_path):
		# create a comp object and specify a path to a tox file
		
		comp = tp.Comp(tox_path)

		comp.set_on_frame_callback(self.on_frame, self)

		comp.start()
		comp.unload()
		pass

# create an instance of a class that runs the comp
example = ExampleRunComp()

# run the comp
example.runComp('tox/yolo.tox')

