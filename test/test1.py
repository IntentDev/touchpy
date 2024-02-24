import sys
import os
import keyboard

# get the path: ../out/build/x64-release
path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-release'))
print(path)
# add the path to the python path
sys.path.append(path)

import tdpy

if __name__ == '__main__':

	tox_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'TopChopDatIO.tox'))

	tox = tdpy.Comp(tox_path)

	while not (keyboard.is_pressed('q')):
		tox.update()

		


		
	
	

	

	