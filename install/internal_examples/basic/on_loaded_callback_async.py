import keyboard
import numpy as np
import torch
import concurrent.futures
import asyncio
import time


import touchpy as tp

tp.init_logging(level=tp.LogLevel.DEBUG)


class MyComp (tp.Comp):
	def __init__(self, flags=tp.CompFlags.INTERNAL_TIME_ASYNC | tp.CompFlags.CUDA_STREAM_DEFAULT, device=1):
		super().__init__(flags=flags, device=device)
		self.device = torch.device(f"cuda:{device}")
		self.frame = 0
		
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

	def on_layout_change(self, info):
		# print('layout change:', info)
		pass

	def on_w_key(self):
		if 'Openwindow' in self.par.names:
			self.par['Openwindow'].pulse()

	def on_frame(self, info):

		self.start_next_frame()
		self.frame += 1

future = concurrent.futures.Future()
def on_loaded(info):
	future.set_result(True)

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
comp.load_tox('TopChopDatIO.tox')

keyboard.add_hotkey('ctrl+w', comp.on_w_key)

result = future.result()
comp.start()

while not keyboard.is_pressed('ctrl+q'):
	pass

comp.stop()
time.sleep(0.5)
comp.unload()
# need to remove all references to the comp object to allow it to be garbage collected
keyboard.remove_all_hotkeys()
del comp




