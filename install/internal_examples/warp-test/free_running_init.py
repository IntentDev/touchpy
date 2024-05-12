import touchpy as tp
import numpy as np


comp = tp.Comp('TopChopDatIO.tox', flags=tp.CompFlags.INTERNAL_TIME_ASYNC)

def on_layout_change(comp, info):
	print('layout changed:')
	print('in tops:', comp.in_tops.count, comp.in_tops.names)
	print('out tops:', comp.out_tops.count, comp.out_tops.names)
	print('in chops:', comp.in_chops.count, comp.in_chops.names)
	print('out chops:', comp.out_chops.count, comp.out_chops.names)
	print('in dats:', comp.in_dats.count, comp.in_dats.names)
	print('out dats:', comp.out_dats.count, comp.out_dats.names)
	print('pars:', comp.par.count, comp.par.names)
	
comp.set_on_layout_change_callback(on_layout_change, {})

comp.start()
comp.par['Monitortop'].val = 'topOut3'
comp.par['Rgba'].set(1.0, .2, .2, 1.0)

# def on_frame(comp, user_data):
# 	comp.in_chops[2].from_numpy(user_data['array'], user_data['names'])
# 	user_data['array'][0,0] += 1

# 	cudamem = comp.out_tops[0].cuda_memory()
# 	comp.in_tops[0].copy_cuda_memory(cudamem)

# 	comp.par['Rotate'].val = comp.par['Rotate'].val + 1

# 	comp.start_next_frame()

# test_array = np.array([[0],[2],[3],[4]], dtype=np.float32)
# test_array_chan_names = ['frame', 'chan2', 'chan3', 'chan4']
# user_data = {'array': test_array, 'names': test_array_chan_names}




