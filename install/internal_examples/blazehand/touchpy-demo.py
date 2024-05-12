import keyboard
import numpy as np
import torch
import cv2
import sys
from pathlib import Path
import os


from blazebase import resize_pad, denormalize_detections
from blazepalm import BlazePalm
from blazehand_landmark import BlazeHandLandmark

from visualization import draw_detections, draw_landmarks, draw_roi, HAND_CONNECTIONS




gpu = torch.device("cuda:0" if torch.cuda.is_available() else "cpu")
torch.set_grad_enabled(False)


localImportPath = Path(__file__).parents[2] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp


class ExampleRunComp:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0
		self.inputBuffer = None
		self.cpuBuffer = None
		self.outBuffer = None
		self.palm_detector = BlazePalm().to(gpu)
		self.palm_detector.load_weights("models/blazepalm.pth")
		self.palm_detector.load_anchors("models/anchors_palm.npy")
		self.palm_detector.min_score_thresh = .75
		self.hand_regressor = BlazeHandLandmark().to(gpu)
		self.hand_regressor.load_weights("models/blazehand_landmark.pth")

	@staticmethod
	def on_layout_change(comp, info):
		comp.out_tops[0].set_cuda_flags(tp.CudaFlags.BGR | tp.CudaFlags.HWC)

	@staticmethod
	def on_frame(comp, this):
		this.frame += 1
		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		
		#read Out TOP from the tox we loaded
		this.inputBuffer = comp.out_tops[0].as_tensor()
		
		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	
		
		#print(this.inputBuffer)
		
		this.cpuBuffer = this.inputBuffer.cpu().numpy()

		
		#this.stream.synchronize()
		print(this.cpuBuffer.shape)
		print(this.cpuBuffer)

		img1, img2, scale, pad = resize_pad(this.cpuBuffer)


		normalized_palm_detections = this.palm_detector.predict_on_image(img1)

		palm_detections = denormalize_detections(normalized_palm_detections, scale, pad)

		xc, yc, scale, theta = this.palm_detector.detection2roi(palm_detections.cpu())
		img, affine2, box2 = this.hand_regressor.extract_roi(this.cpuBuffer, xc, yc, theta, scale)
		flags2, handed2, normalized_landmarks2 = this.hand_regressor(img.to(gpu))
		landmarks2 = this.hand_regressor.denormalize_landmarks(normalized_landmarks2.cpu(), affine2)

		for i in range(len(flags2)):
			landmark, flag = landmarks2[i], flags2[i]
			if flag>.5:
				draw_landmarks(this.cpuBuffer, landmark[:,:2], HAND_CONNECTIONS, size=2)

		this.outBuffer = torch.from_numpy(this.cpuBuffer).cuda()
		comp.in_tops[0].from_tensor(this.outBuffer, flags=tp.CudaFlags.BGR)
		

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

