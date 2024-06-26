import keyboard
import numpy as np
import torch
import concurrent.futures

import touchpy as tp

tp.init_logging(level=tp.LogLevel.DEBUG)

td_path = 'C:/Program Files/Derivative/TouchDesigner.2023.31253.28'
# td_path = 'C:/Program Files/Derivative/TouchDesigner.2023.11764.22'

class MyComp (tp.Comp):
	def __init__(self, flags=tp.CompFlags.INTERNAL_TIME_AUTO | tp.CompFlags.CUDA_STREAM_DEFAULT, device=0, td_path=td_path):
	# def __init__(self, flags=tp.CompFlagBits.INTERNAL_TIME_AUTO | tp.CompFlagBits.CUDA_STREAM_DEFAULT, device=0, td_path=td_path):
	# def __init__(self, flags=tp.CompFlags(tp.CompFlagBits.INTERNAL_TIME_AUTO | tp.CompFlagBits.CUDA_STREAM_DEFAULT), device=0, td_path=td_path):
		super().__init__(flags=flags, device=device, td_path=td_path)
		self.device = torch.device(f"cuda:{device}")
		self.frame = 0
		
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

	def on_layout_change(self, info):
		self.out_tops[0].set_cuda_flags(tp.CudaFlags.RGB | tp.CudaFlags.HWC)
		pass

	def on_w_key(self):
		if 'Openwindow' in self.par.names:
			self.par['Openwindow'].pulse()

	def ctrl_q_key_is_pressed(self) -> bool:
		if (keyboard.is_pressed('ctrl+q')):
			self.stop()
			self.unload()
			return True
		return False

	def on_frame(self, info):
		if self.ctrl_q_key_is_pressed(): # need to exit here for clean exit
			return
		
		tensor = self.out_tops[0].as_tensor()
	
		self.start_next_frame()

		self.in_tops[0].from_tensor(tensor, flags=tp.CudaFlags.RGB | tp.CudaFlags.HWC)
		
		self.frame += 1

future_load = concurrent.futures.Future()
def on_loaded(info):
	future_load.set_result(True)

# # load comp in local scope to allow garbage collection
# # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# def run():
# 	comp = MyComp()
# 	comp.set_on_loaded_callback(on_loaded, {})
# 	comp.load_tox('TopChopDatIO.tox')

# 	keyboard.add_hotkey('ctrl+w', comp.on_w_key)

# 	result = future.result()
# 	comp.start()

# 	# need to remove all references to the comp object to allow it to be garbage collected
# 	keyboard.remove_all_hotkeys()

# run()

# load comp in global scope and manually delete it to allow garbage collection
# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
comp = MyComp()
comp.set_on_loaded_callback(on_loaded, {})
comp.load('TopChopDatIO.tox')

keyboard.add_hotkey('ctrl+w', comp.on_w_key)

result = future_load.result()
print('on_layout_change()', 'file_path:', comp.file_path, 'td_path:', comp.td_path, 'device:', comp.cuda_device, 'flags:', comp.flags)
comp.start()

# need to remove all references to the comp object to allow it to be garbage collected
keyboard.remove_all_hotkeys()

# del comp




