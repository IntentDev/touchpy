# Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

###########################################################################
# Example Sim Granular
#
# Shows how to set up a particle-based granular material model using the
# wp.sim.ModelBuilder().
#
###########################################################################

import warp as wp
import warp.sim
import warp.sim.render

import touchpy as tp
import keyboard

wp.init()


class Example:
	def __init__(self, stage):
		self.frame_dt = 1.0 / 60
		self.frame_count = 1
		self.frameTD = 0

		self.sim_substeps = 64
		self.sim_dt = self.frame_dt / self.sim_substeps
		self.sim_steps = self.frame_count * self.sim_substeps
		self.sim_time = 0.0

		self.radius = 0.1

		builder = wp.sim.ModelBuilder()
		builder.default_particle_radius = self.radius

		builder.add_particle_grid(
			dim_x=16,
			dim_y=32,
			dim_z=16,
			cell_x=self.radius * 2.0,
			cell_y=self.radius * 2.0,
			cell_z=self.radius * 2.0,
			pos=wp.vec3(0.0, 1.0, 0.0),
			rot=wp.quat_identity(),
			vel=wp.vec3(5.0, 0.0, 0.0),
			mass=0.1,
			jitter=self.radius * 0.1,
		)

		self.model = builder.finalize()
		self.model.particle_kf = 25.0

		self.model.soft_contact_kd = 100.0
		self.model.soft_contact_kf *= 2.0

		self.state_0 = self.model.state()
		self.state_1 = self.model.state()

		self.integrator = wp.sim.SemiImplicitIntegrator()

		self.renderer = None
		if stage:
			self.renderer = wp.sim.render.SimRenderer(self.model, stage, scaling=20.0)

		self.use_graph = wp.get_device().is_cuda
		if self.use_graph:
			with wp.ScopedCapture() as capture:
				self.simulate()
			self.graph = capture.graph

	def simulate(self):
		for _ in range(self.sim_substeps):
			self.state_0.clear_forces()
			self.integrator.simulate(self.model, self.state_0, self.state_1, self.sim_dt)

			# swap states
			(self.state_0, self.state_1) = (self.state_1, self.state_0)

	def step(self):
		with wp.ScopedTimer("step", active=True):
			self.model.particle_grid.build(self.state_0.particle_q, self.radius * 2.0)
			if self.use_graph:
				wp.capture_launch(self.graph)
			else:
				self.simulate()

		self.sim_time += self.frame_dt

	def render(self):
		if self.renderer is None:
			return

		with wp.ScopedTimer("render", active=True):
			self.renderer.begin_frame(self.sim_time)
			self.renderer.render(self.state_0)
			self.renderer.end_frame()

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
		
		# if (keyboard.is_pressed('r')):
		# 	this.reset_particles()
		
			
		# if this.frameTD == 10:
		# 	y = this.state_0.particle_q.reshape((128, 64))
		# 	print(y.shape, y.strides, y.dtype)
		# 	dl = wp.to_dlpack(y)
		# 	info = tp.get_dlpack_capsule_info(dl)
		# 	print(info)

		# 	comp.stop()


		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	

		this.step()
		y = this.state_0.particle_q.reshape((128, 64))
		comp.in_tops[0].from_dlpack(wp.to_dlpack(y), tp.CudaFlags.RGB)

		this.frameTD += 1

if __name__ == "__main__":
	stage_path = "example_granular.usd"

	example = Example(stage_path)

	example.runComp('tox/yolo.tox')

	# for _ in range(example.frame_count):
	# 	example.step()
	# 	example.render()

	# print(example.state_0.particle_q.shape, example.state_0.particle_q.strides, example.state_0.particle_q.dtype)

	# if example.renderer:
	# 	example.renderer.save()
