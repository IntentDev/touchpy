import pytest
import pathlib
import sys

localImportPath = pathlib.Path(__file__).parents[2] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

class TestClass:
	def __init__(self):
		self.running = True # used to gracefully exit the loop
		self.frame = 0
		self.runComp('../tox/test.tox')
		
	@staticmethod
	def on_frame(comp, this):

		# print out all the parameters name on the first frame
		parNames = comp.par.names()
		if (this.frame == 0):
			for name in parNames:
				print(f"{name}: {comp.par[name].val}")
			
		
		# comp.start_next_frame() is only needed if comp.start(start_next_frame=False)
		# or comp.update(call_start_next_frame=False), they are both False by default
		#
		# this allows the user to control when the next frame starts and so they can do work on non-comp members
		# if comp.start(True) or comp.update(True) is called, start_next_frame() will be called automatically
		# and shouldn't be called here
		if comp.start_next_frame():
			# do work on non-comp members for the next frame here
			pass
		else:
			# we have a problem with TouchEngine starting the next frame.
			# Currently the error will be printed in the console but start_next_frame()
			# will need to be updated to return an enum with the result for error handling
			# in the future
			pass

		this.frame += 1

	def runComp(self, tox_path):
		# create a comp object and specify a path to a tox file
		comp = tp.Comp(tox_path)

		# set the on_frame callback
		# first are is static method (or free function) and second is the object to be passed to the callback
		# in this case, self is passed to the callback so we can access the class members directly
		comp.set_on_frame_callback(self.on_frame, self)

	
		# this runs a loop that calls update() internally
		#comp.start() # need to manually call start_next_frame() in on_frame callback to start next frame
		comp.start(True) # update function starts next frame automatically

		# run the loop in Python anc call update manually
		# while not (keyboard.is_pressed('q')):
		#	# comp.update(True) # update function starts next frame
		# 	comp.update() #  need to manually call start_next_frame() in on_frame callback to start next frame
		pass

# create an instance of a class that runs the comp
test = TestClass()

