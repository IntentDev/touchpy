import sys
import os
import keyboard

# get the path to touchpy.pyd: ../out/build/x64-release
path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-release'))
# path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-relwithdebuginfo'))
sys.path.append(path)

import touchpy as tp

class ExampleRunComp:
	def __init__(self):
		self.frame = 0
		self.a = 1
		self.b = 2

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q')):
			comp.stop()
			return

		comp.input_dats[0].set_string(f"Hello World! frame: {this.frame}")
		comp.input_dats['datIn2'].set_string(f"Hello Again! frame: {this.frame}")

		this.frame += 1

	def runComp(self, tox_path):
		comp = tp.Comp(tox_path)
		comp.set_on_frame_callback(self.on_frame, self)
		comp.start()

		# del(comp)
		# while not (keyboard.is_pressed('q')):
		# 	comp.update()
		pass


if __name__ == '__main__':

	example = ExampleRunComp()
	tox_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'TopChopDatIO.tox'))
	example.runComp(tox_path)

	pass

	
	

	

	