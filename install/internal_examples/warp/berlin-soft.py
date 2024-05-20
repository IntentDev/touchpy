


import sys
import warp as wp
import warp.sim
import numpy as np
import igl
from pxr import Vt

from pathlib import Path
localImportPath = Path.cwd().parents[1] / 'modules'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp
import keyboard
wp.init()




@wp.kernel(enable_backward=False)
def reset_softbody_points(
	input_points: wp.array(dtype=wp.vec3),
	out_points: wp.array(dtype=wp.vec3),
	
):
	tid = wp.tid()
	point = input_points[tid]
	out_points[tid] = point



class Example:
	def __init__(self):
		self.inititalize()

	def inititalize(self):
		fps = 60
		self.frame_dt = 1.0 / fps
		self.sim_substeps = 32
		self.sim_dt = self.frame_dt / self.sim_substeps
		self.sim_time = 0.0
		self.sim_iterations = 1
		self.sim_relaxation = 1.0
		self.profiler = {}

		builder = wp.sim.ModelBuilder()
		builder.default_particle_radius = 0.01

		builder.add_soft_grid(
			pos=wp.vec3(0.0, 0.0, 0.0),
			rot=wp.quat_identity(),
			vel=wp.vec3(0.0, 0.0, 0.0),
			dim_x=25,
			dim_y=10,
			dim_z=10,
			cell_x=0.1,
			cell_y=0.1,
			cell_z=0.1,
			density=100.0,
			k_mu=50000.0,
			k_lambda=20000.0,
			k_damp=0.0,
		)

		b = builder.add_body(origin=wp.transform((0.5, 2.5, 0.5), wp.quat_identity()))
		builder.add_shape_sphere(body=b, radius=0.75, density=100.0)

		self.model = builder.finalize()

		self.all_start_points = self.model.state().particle_q
		self.softbody_start_points = self.model.state().particle_q[0:2500]
		self.rigidbody_start_points = self.model.state().particle_q[2500:]
		print(self.softbody_start_points.shape)

		self.model.ground = True
		self.model.soft_contact_ke = 1.0e3
		self.model.soft_contact_kd = 0.0
		self.model.soft_contact_kf = 1.0e3

		self.integrator = wp.sim.SemiImplicitIntegrator()

		self.state_0 = self.model.state()
		self.state_1 = self.model.state()

		
		self.use_cuda_graph = wp.get_device().is_cuda
		if self.use_cuda_graph:
			with wp.ScopedCapture() as capture:
				self.simulate()
			self.graph = capture.graph
			

	def simulate(self):
		for _s in range(self.sim_substeps):
			wp.sim.collide(self.model, self.state_0)

			self.state_0.clear_forces()
			self.state_1.clear_forces()

			self.integrator.simulate(self.model, self.state_0, self.state_1, self.sim_dt)

			# swap states
			(self.state_0, self.state_1) = (self.state_1, self.state_0)

	def step(self):
		with wp.ScopedTimer("step", dict=self.profiler):
			if self.use_cuda_graph:
				wp.capture_launch(self.graph)
			else:
				self.simulate()
			self.sim_time += self.frame_dt

	def restart(self):
		wp.launch(
		kernel=reset_softbody_points,
			dim=len(self.state_0.particle_q),
			inputs=[
				self.all_start_points,
			],
			outputs=[
				self.state_0.particle_q
			],
		)

	def print(self):
		print(self.mesh_points.shape)	

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
		
		if (keyboard.is_pressed('i')):
			this.initialize()


		if (keyboard.is_pressed('r')):
			this.restart()
		
		if (keyboard.is_pressed('p')):
			this.print()
			
	
		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	

		this.step()
		softbody = this.state_0.particle_q[0:2500]
		y = softbody.reshape((50, 50))
		
		comp.in_tops[0].from_dlpack(wp.to_dlpack(y), tp.CudaFlags.RGB)

		


if __name__ == "__main__":
	

	example = Example()
	example.runComp('tox/yolo.tox')
	#for i in range(example.sim_frames):
	#	example.step()
	#	example.render()

	#frame_times = example.profiler["step"]
	#print("\nAverage frame sim time: {:.2f} ms".format(sum(frame_times) / len(frame_times)))

	#if example.renderer:
	#	example.renderer.save()