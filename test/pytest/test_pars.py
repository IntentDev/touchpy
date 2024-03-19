import pytest
from pathlib import Path
import sys
import keyboard
import json

localImportPath = Path(__file__).parents[2] / 'out/build/x64-release'
if str(localImportPath) not in sys.path:
	sys.path.insert(0,str(localImportPath))	

import touchpy as tp

class TestClass:
	
	@classmethod
	def setup_class(cls):
		"""Setup any state specific to the execution of the given class.
		"""
		print("setup class...")
		test = TestClass()
		cls.frame = 0
		cls.done = False
		cls.runComp(cls, '../tox/test.tox')
		cls.referenceFile = Path(__file__).parents[1] / 'tox/parvalues.json'
		cls.referenceValues = json.loads(cls.referenceFile.read_text())
	
	@classmethod
	def teardown_class(cls):
		""""Teardown any state that was previously setup with a setup_class method.
		"""
		print("stopping class")
		cls.done = True


	def test_float(self):
		parname = "Float"
		print(f"testing {parname}...")
		val = self.comp.par[parname].val
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_float2(self):
		parname = "Float2"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].x, self.comp.par[parname].y]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_float3(self):
		parname = "Float3"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].x, self.comp.par[parname].y, self.comp.par[parname].z]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_float4(self):
		parname = "Float4"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].x, self.comp.par[parname].y, self.comp.par[parname].z, self.comp.par[parname].w]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_int(self):
		parname = "Int"
		print(f"testing {parname}...")
		val = self.comp.par[parname].val
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref
	
	def test_int2(self):
		parname = "Int2"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].x, self.comp.par[parname].y]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_int3(self):
		parname = "Int3"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].x, self.comp.par[parname].y, self.comp.par[parname].z]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_int4(self):
		parname = "Int4"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].x, self.comp.par[parname].y, self.comp.par[parname].z, self.comp.par[parname].w]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_rgb(self):
		parname = "Rgb"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].r, self.comp.par[parname].g, self.comp.par[parname].b]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_rgba(self):
		parname = "Rgba"
		print(f"testing {parname}...")
		val = [self.comp.par[parname].r, self.comp.par[parname].g, self.comp.par[parname].b, self.comp.par[parname].a]
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref

	def test_string(self):
		parname = "Str"
		print(f"testing {parname}...")
		val = self.comp.par[parname].val
		ref = self.referenceValues["Test"][parname]["val"]
		assert val == ref


	
	@staticmethod
	def on_frame(comp, this):

		# optional used to quit if comp.start() is called
		if (keyboard.is_pressed('q')):
			print("pressed q key")
			comp.stop()
			return
		
		if this.done == True:
			print("done is true")
			comp.stop()
			return
		
		#if comp.start_next_frame():
			# do work on non-comp members for the next frame here
		#	pass
		#else:
			# we have a problem with TouchEngine starting the next frame.
			# Currently the error will be printed in the console but start_next_frame()
			# will need to be updated to return an enum with the result for error handling
			# in the future
			#pass

		
		this.frame += 1
		

	def runComp(self, tox_path):
		print("starting tox...")
		# create a comp object and specify a path to a tox file
		self.comp = tp.Comp(tox_path)

		# set the on_frame callback
		# first are is static method (or free function) and second is the object to be passed to the callback
		# in this case, self is passed to the callback so we can access the class members directly
		self.comp.set_on_frame_callback(self.on_frame, self)

	
		# this runs a loop that calls update() internally
		#self.comp.start() # need to manually call start_next_frame() in on_frame callback to start next frame
		self.comp.start(True) # update function starts next frame automatically

		

# create an instance of a class that runs the comp
#test = TestClass()

