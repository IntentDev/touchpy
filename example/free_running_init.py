import touchpy as tp
import numpy as np

test_array = np.array([[1],[2],[3],[4]], dtype=np.float32)
test_array_chan_names = ['frame', 'chan2', 'chan3', 'chan4']

user_data = {'array': test_array, 'names': test_array_chan_names}

def on_frame(comp, user_data):
	test_array[0,0] = user_data['frame']

	comp.in_chops[2].from_numpy(test_array, test_array_chan_names)
	
	user_data['frame'] += 1
	# print('Called on_frame()')
	pass


