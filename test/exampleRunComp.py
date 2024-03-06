import sys
import os
import keyboard
import numpy as np

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
		self.test_array = np.array([[1],[2],[3],[4],[5],[6],[7],[8],[9],[10]], dtype=np.float32)
		self.test_array_chan_names = [f"channel{i}" for i in range(10)]

	def test(self, chans):
		print(chans)
		pass

	@staticmethod
	def on_frame(comp, this):

		if (keyboard.is_pressed('q')):
			comp.stop()
			return

		comp.input_chops[0].from_numpy(this.test_array)
		this.test_array += 1

		# get a reference to the numpy array, some functions that do not copy 
		# will not work with this passed as an argument, such as ChopLink.from_numpy() 
		# faster than a copy though for large arrays... good for read only operations
		chans1_ref = comp.output_chops['chopOut1'].as_numpy_ref() 

		chans1 = comp.output_chops[1].as_numpy()
		comp.input_chops[1].from_numpy(chans1)

		chans2 = comp.output_chops[2].as_numpy()
		print(chans2)


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

	
	

	

	