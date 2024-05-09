# Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

###########################################################################
# Example Sim Cloth
#
# Shows a simulation of an FEM cloth model colliding against a static
# rigid body mesh using the wp.sim.ModelBuilder().
#
###########################################################################
import time




import math
import numpy as np
import os
from enum import Enum
from pxr import Usd, UsdGeom
import sys
import warp as wp
import warp.sim
from pathlib import Path
localImportPath = Path.cwd().parents[1] / 'modules'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp
import keyboard
wp.init()


class IntegratorType(Enum):
	EULER = "euler"
	XPBD = "xpbd"

	def __str__(self):
		return self.value


class Example:
	def __init__(self, stage, integrator=IntegratorType.EULER):
		self.integrator_type = integrator
		self.profiler = {}
		self.reset()
		
	def reset(self):	
		
		
		self.sim_width = 49
		self.sim_height = 49

		self.sim_fps = 60.0
		self.sim_substeps = 32
		self.sim_duration = 5.0
		self.sim_frames = int(self.sim_duration * self.sim_fps)
		self.frame_dt = 1.0 / self.sim_fps
		self.sim_dt = self.frame_dt / self.sim_substeps
		self.sim_time = 0.0
		
		

		
		builder = wp.sim.ModelBuilder()

		
		

		if self.integrator_type == IntegratorType.EULER:
			builder.add_cloth_grid(
				pos=wp.vec3(0, 7.0, 0),
				rot=wp.quat_from_axis_angle(wp.vec3(1.0, 0.0, 0.0), math.pi * 0.5),
				vel=wp.vec3(0.0, 0.0, 0.0),
				dim_x=self.sim_width,
				dim_y=self.sim_height,
				cell_x=0.1,
				cell_y=0.1,
				mass=0.1,
				tri_ke=1.0e3,
				tri_ka=1.01e3,
				tri_kd=1.e1,
				tri_drag=0.8,
				edge_ke=0.2
			)
		else:
			builder.add_cloth_grid(
				pos=wp.vec3(0.0, 7.0, 0.0),
				rot=wp.quat_from_axis_angle(wp.vec3(1.0, 0.0, 0.0), math.pi * 0.5),
				vel=wp.vec3(0.0, 0.0, 0.0),
				dim_x=self.sim_width,
				dim_y=self.sim_height,
				cell_x=0.15,
				cell_y=0.15,
				mass=0.2,
				edge_ke=2.0e2,
				add_springs=True,
				spring_ke=0.5e3,
				spring_kd=1.0,
				tri_drag=1.0
			)

	  
		
		

		builder.add_shape_sphere(
			body=-1, 
			pos=(2.5, 5.0, 2.5), 
			rot=(0.0, 0.0, 0.0, 1.0), 
			radius=0.75, 
			density=None, 
			ke=1.0e2, 
			kd=1.0e2, 
			kf=100.0e6,
			mu=0.1,
			)	


		#end_time = time.time()
		#elapsed_time = end_time - start_time
		#print(elapsed_time)



		if self.integrator_type == IntegratorType.EULER:
			self.integrator = wp.sim.SemiImplicitIntegrator()
		else:
			self.integrator = wp.sim.XPBDIntegrator(iterations=1)

		self.model = builder.finalize()
		self.model.ground = True
		self.model.soft_contact_ke = 1.0e4
		self.model.soft_contact_kd = 1.0e2

		self.state_0 = self.model.state()
		self.state_1 = self.model.state()

	
		
		self.use_graph = wp.get_device().is_cuda
		if self.use_graph:
			with wp.ScopedCapture() as capture:
				self.simulate()
			self.graph = capture.graph



	def simulate(self):
		wp.sim.collide(self.model, self.state_0)

		for _ in range(self.sim_substeps):
			self.state_0.clear_forces()

			self.integrator.simulate(self.model, self.state_0, self.state_1, self.sim_dt)

			# swap states
			(self.state_0, self.state_1) = (self.state_1, self.state_0)

	def step(self):
		#with wp.ScopedTimer("step", dict=self.profiler):
		if self.use_graph:
			wp.capture_launch(self.graph)
		else:
			self.simulate()
		
		self.sim_time += self.frame_dt




	def runComp(self, tox_path):
		comp = tp.Comp(tox_path, flags=tp.CompFlags.INTERNAL_TIME_AUTO | tp.CompFlags.CUDA_STREAM_INTERNAL)
		comp.set_on_layout_change_callback(self.on_layout_change, self)
		comp.set_on_frame_callback(self.on_frame, self)
		comp.start()
		comp.unload()
		pass

	
	@staticmethod
	def on_layout_change(comp, this):
		#comp.out_tops[0].set_cuda_flags(tp.CudaFlags.BGR | tp.CudaFlags.HWC)
		this.stream = wp.Stream("cuda:0", cuda_stream=comp.cuda_stream())
		
		pass


	@staticmethod
	def on_frame(comp, this):
		
		if (keyboard.is_pressed('q')):
			comp.stop() # stop running the comp
			return
		
		if (keyboard.is_pressed('r')):
			this.reset()
		
			
	
		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	

		this.step()
		y = this.state_0.particle_q
		y = this.state_0.particle_q.reshape((this.sim_width+1, this.sim_height+1))
		comp.in_tops[0].from_dlpack(wp.to_dlpack(y), tp.CudaFlags.RGB)

		


if __name__ == "__main__":
	import argparse

	parser = argparse.ArgumentParser()
	parser.add_argument(
		"--integrator",
		help="Type of integrator",
		type=IntegratorType,
		choices=list(IntegratorType),
		default=IntegratorType.EULER,
	)

	args = parser.parse_args()

	stage_path = "example_cloth.usd"

	example = Example(stage_path, integrator=args.integrator)
	example.runComp('tox/yolo.tox')
	#for i in range(example.sim_frames):
	#	example.step()
	#	example.render()

	#frame_times = example.profiler["step"]
	#print("\nAverage frame sim time: {:.2f} ms".format(sum(frame_times) / len(frame_times)))

	#if example.renderer:
	#	example.renderer.save()