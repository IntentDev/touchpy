import touchpy as tp
import modules.utils as utils
import math

class MyComp (tp.Comp):
	def __init__(self):
		super().__init__()
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

		self.frame = 0
	
	def on_layout_change(self, info):
		print('layout changed:')
		print('pars:\n', *[f"\t{name}\n" for name in comp.par.names])

		# pulse a par
		if 'Openwindow' in self.par.names:
			# only works on supported TD builds
			# official build with support should be available soon
			self.par['Openwindow'].pulse()

	def on_frame(self, info):
		if utils.check_key('q'):
			self.stop()
			return

		# it's typically a good idea to call start_next_frame() at the beginning of the 
		# on_frame method before doing any processing and setting of data but it's not required
		self.start_next_frame()

		# get pars
		scale = self.par['Scale']
		translate = self.par['Translate']

		# get a par value
		if self.frame % 60 == 0:
			# printing is slow...
			# print('Scale:', scale.val, ', Translate:', translate.val, translate.get())
			pass

		# pars can be set with the .val member or the .set() method
		scale.val = math.sin(self.frame * 0.01) * 0.25 + 0.75
		self.par['Rotate'].set(-self.frame * 0.2)

		# pars with multiple components can be accessed with the .x, .y, .z, .w members
		theta = self.frame * 0.005
		radius = 0.2
		x = radius * math.cos(theta)
		y = radius * math.sin(theta)

		# get/set by component member, although it's better (faster) to set all the 
		# # components at once with .val or .set()
		translate.x = x
		translate.y = y

		# get/set by val member
		my_float2 = tp.Float2(x, y)
		translate.val = my_float2

		# set with the .set() method
		translate.set(x, y)
		translate.set((x, y))
		translate.set([x, y])
		translate.set(my_float2)

		# color pars can be accessed with the .r, .g, .b, .a members
		color = self.par['Rgba']

		# best to set all the values at once
		my_color = tp.Color(1, math.sin(.5 + self.frame * .1) * 0.5 + 0.5, math.sin(self.frame * .1) * 0.5 + 0.5, 1)
		color.val = my_color

		# valid par types are: 
			# bool, 
			# float, 
			# int, 
			# str, 
			# tp.Float2, 
			# tp.Float3, 
			# tp.Float4, 
			# tp.Color, 
			# tp.Int2, 
			# tp.Int3, 
			# tp.Int4

		# currently the only exposed functionality is getting and setting members of multi-component pars
		# in the future more functionality will be exposed, especially for the tp.Color type
		
		self.frame += 1

comp = MyComp()
comp.load('TopChopDatIO.tox') 
comp.start()
comp.unload()






