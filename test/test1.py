import sys
import os
import keyboard

# get the path to touchpy.pyd: ../out/build/x64-release
path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-release'))
sys.path.append(path)

import touchpy as tp

def on_frame_start(comp, user_data):
	if (keyboard.is_pressed('q')):
		comp.stop()

	comp.input_dats[0].set_string(f"Hello World! frame: {user_data['frame']}")
	comp.input_dats['datIn2'].set_string(f"Hello Again! frame: {user_data['frame']}")
	# print(user_data)
	user_data['frame'] += 1


if __name__ == '__main__':

	tox_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'TopChopDatIO.tox'))

	test = {}
	test['frame'] = 0
	test['a'] = 1
	test['b'] = 2
	comp = tp.Comp(tox_path)
	comp.set_on_frame_start_callback(on_frame_start, test)
	comp.start()

	# while not (keyboard.is_pressed('q')):
	# 	comp.update()
	pass

		


		
	
	

	

	