# to use:
# 	open command prompt in the directory of this file
# 	run: python
# 	run: from free_running_init import *
# 	run: comp = tp.Comp('FreeRunningIO.tox', free_running=True) # or any other tox file
# 	run: comp.set_on_frame_callback(on_frame, user_data)
# 	set or get values on the comp
# 	run: del(comp) or exit() python

import touchpy as tp
import numpy as np

def on_frame(comp, user_data):
	comp.in_chops[2].from_numpy(user_data['array'], user_data['names'])
	user_data['array'][0,0] += 1
	pass

test_array = np.array([[0],[2],[3],[4]], dtype=np.float32)
test_array_chan_names = ['frame', 'chan2', 'chan3', 'chan4']
user_data = {'array': test_array, 'names': test_array_chan_names}


comp = tp.Comp('FreeRunningIO.tox')

comp.set_on_frame_callback(on_frame, user_data)

i = 0

while i < 100:
	comp.update()
	i += 1




