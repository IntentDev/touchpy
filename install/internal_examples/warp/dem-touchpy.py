
# imports
import numpy as np
import keyboard
import warp as wp
from pathlib import Path
import sys
import torch
import warp.render
import matplotlib.pyplot as plt

localImportPath = Path.cwd().parents[1] / 'modules'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp
wp.init()


torch.set_printoptions(precision=10, sci_mode=False)

@wp.func
def contact_force(n: wp.vec3, v: wp.vec3, c: float, k_n: float, k_d: float, k_f: float, k_mu: float):
	vn = wp.dot(n, v)
	jn = c * k_n
	jd = min(vn, 0.0) * k_d

	# contact force
	fn = jn + jd

	# friction force
	vt = v - n * vn
	vs = wp.length(vt)

	if vs > 0.0:
		vt = vt / vs

	# Coulomb condition
	ft = wp.min(vs * k_f, k_mu * wp.abs(fn))

	# total force
	return -n * fn - vt * ft


@wp.kernel
def apply_forces(
	grid: wp.uint64,
	particle_x: wp.array(dtype=wp.vec3),
	particle_v: wp.array(dtype=wp.vec3),
	particle_f: wp.array(dtype=wp.vec3),
	radius: float,
	k_contact: float,
	k_damp: float,
	k_friction: float,
	k_mu: float,
):
	tid = wp.tid()

	# order threads by cell
	i = wp.hash_grid_point_id(grid, tid)

	x = particle_x[i]
	v = particle_v[i]

	f = wp.vec3()

	# ground contact
	n = wp.vec3(0.0, 1.0, 0.0)
	c = wp.dot(n, x)

	cohesion_ground = 0.02
	cohesion_particle = 0.0055

	if c < cohesion_ground:
		f = f + contact_force(n, v, c, k_contact, k_damp, 100.0, 0.5)

	# particle contact
	neighbors = wp.hash_grid_query(grid, x, radius * 5.0)

	for index in neighbors:
		if index != i:
			# compute distance to point
			n = x - particle_x[index]
			d = wp.length(n)
			err = d - radius * 2.0

			if err <= cohesion_particle:
				n = n / d
				vrel = v - particle_v[index]

				f = f + contact_force(n, vrel, err, k_contact, k_damp, k_friction, k_mu)

	particle_f[i] = f


@wp.kernel
def integrate(
	x: wp.array(dtype=wp.vec3),
	v: wp.array(dtype=wp.vec3),
	f: wp.array(dtype=wp.vec3),
	gravity: wp.vec3,
	dt: float,
	inv_mass: float,
):
	tid = wp.tid()

	v_new = v[tid] + f[tid] * inv_mass * dt + gravity * dt
	x_new = x[tid] + v_new * dt

	v[tid] = v_new
	x[tid] = x_new


class Example:
	def __init__(self):
		
		self.buffer = None
		self.frameTD = 0
		self.frame_dt = 1.0 / 60
		self.frame_count = 1000

		self.sim_substeps = 64
		self.sim_dt = self.frame_dt / self.sim_substeps
		self.sim_steps = self.frame_count * self.sim_substeps
		self.sim_time = 0.0

		self.point_radius = 0.1

		self.k_contact = 8000.0
		self.k_damp = 2.0
		self.k_friction = 1.0
		self.k_mu = 100000.0  # for cohesive materials


		self.inv_mass = 64.0

		self.grid = wp.HashGrid(128, 128, 128)
		self.grid_cell_size = self.point_radius * 5.0

		
		self.reset_particles()

		
		#self.renderer = wp.render.OpenGLRenderer(screen_width=1024, screen_height=768,camera_pos=(-10.0, 2.0, 40.0), camera_front=(-1.0, 0.0, 1.0),vsync=False)
		#self.renderer.render_ground()

		#self.fig = plt.figure(1)
		#self.pixels = wp.zeros((self.renderer.screen_height, self.renderer.screen_width, 3), dtype=wp.float32)

		self.use_graph = wp.get_device().is_cuda
		if self.use_graph:
			with wp.ScopedCapture() as capture:
				self.simulate()
			self.graph = capture.graph

	def reset_particles(self):
		self.points = self.particle_grid(10, 100, 10, (0.0, 0.5, 0.0), self.point_radius, 0.1)

		self.x = wp.array(self.points, dtype=wp.vec3)
		self.v = wp.array(np.ones([len(self.x), 3]) * np.array([0.0, 0.0, 1.0]), dtype=wp.vec3)
		self.f = wp.zeros_like(self.v)
		self.sim_time = 0.0




	# creates a grid of particles
	def particle_grid(self, dim_x, dim_y, dim_z, lower, radius, jitter):
		points = np.meshgrid(np.linspace(0, dim_x, dim_x), np.linspace(0, dim_y, dim_y), np.linspace(0, dim_z, dim_z))
		points_t = np.array((points[0], points[1], points[2])).T * radius * 2.0 + np.array(lower)
		points_t = points_t + np.random.rand(*points_t.shape) * radius * jitter
		return points_t.reshape((-1, 3))
	

	
	
	
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
			this.reset_particles()
		
			
		if this.frameTD == 300:
			print(this.x.shape, this.x.strides, this.x.dtype)
			y = this.x.reshape((100, 100))
			print(y.shape, y.strides, y.dtype)
			comp.stop()


		######### Process and Copy for next frame (Fast) ###################
		comp.start_next_frame()	

		this.step()
		y = this.x.reshape((100, 100))
		comp.in_tops[0].from_dlpack(wp.to_dlpack(y), tp.CudaFlags.RGB)

		this.frameTD += 1
		pass

	def simulate(self):
		for _ in range(self.sim_substeps):
			wp.launch(
				kernel=apply_forces,
				dim=len(self.x),
				inputs=[
					self.grid.id,
					self.x,
					self.v,
					self.f,
					self.point_radius,
					self.k_contact,
					self.k_damp,
					self.k_friction,
					self.k_mu,
				],
			)
			wp.launch(
				kernel=integrate,
				dim=len(self.x),
				inputs=[self.x, self.v, self.f, (0.0, -9.8, 0.0), self.sim_dt, self.inv_mass],
			)

	def step(self):
		with wp.ScopedTimer("step", print=False, active=True):
			with wp.ScopedTimer("grid build", active=False):
				self.grid.build(self.x, self.grid_cell_size)

			if self.use_graph:
				wp.capture_launch(self.graph)
			else:
				self.simulate()

			self.sim_time += self.frame_dt





if __name__ == "__main__":
	

	example = Example()
	# run the comp
	example.runComp('tox/yolo.tox')

	
