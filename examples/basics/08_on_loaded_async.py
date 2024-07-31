import touchpy as tp
import modules.utils as utils
import concurrent.futures
import time

class MyComp (tp.Comp):
	def __init__(self, flags):
		super().__init__(flags=flags)

		self.frame = 0
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

	def on_layout_change(self, info):
		print('layout changed:')

		if 'Openwindow' in self.par.names:
			self.par['Openwindow'].pulse()

	def on_frame(self, info):
		self.start_next_frame()

		if self.frame % 60 == 0:
			print(f'frame: {self.frame}')

		self.frame += 1

# create a future to signal when the component is loaded
future_load = concurrent.futures.Future()

# define the on_loaded callback that will be called when the component is loaded
def on_loaded(info):
	future_load.set_result(True)

# create a future to signal when the component is stopped
future_stop = concurrent.futures.Future()

# define the on_stop callback that will be called when the component is stopped
def on_stop(info):
	future_stop.set_result(True)

# create a new instance of MyComp and set the ASYNC flag 
# INTERNAL_TIME_ASYNC will load the update function in a separate thread
# which will not block and run asynchronously in the background
comp = MyComp(tp.CompFlags.INTERNAL_TIME_ASYNC)

# set the on_loaded and on_stop callbacks
comp.set_on_loaded_callback(on_loaded, {})
comp.set_on_stop_callback(on_stop, {})

comp.load('TopChopDatIO.tox')

# wait for the component to be loaded
print('waiting for component to be loaded...')
future_load.result()

comp.start()

# wait for the user to press 'q' to continue which will result in 
# comp.stop() being called
print('waiting for user to press "q" to continue...')
while not utils.check_key('q'):
	time.sleep(0.1)

comp.stop()

# wait for the component to be stopped to unload it and clean up
print('waiting for component to be stopped...')
future_stop.result()

comp.unload()






