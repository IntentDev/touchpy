import touchpy as tp
import modules.utils as utils
import concurrent.futures

class MyComp (tp.Comp):
	def __init__(self):
		super().__init__()

		self.frame = 0
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

	def on_layout_change(self, info):
		print('layout changed:')

		if 'Openwindow' in self.par.names:
			self.par['Openwindow'].pulse()

	def on_frame(self, info):
		if utils.check_key('q'):
			self.stop()
			return

		self.start_next_frame()
		self.frame += 1


# create a future to signal when the component is loaded
future_load = concurrent.futures.Future()

# define the on_loaded callback that will be called when the component is loaded
def on_loaded(info):
	future_load.set_result(True)

# create a new instance of MyComp
comp = MyComp()

# set the on_loaded callback
comp.set_on_loaded_callback(on_loaded, {})

# if no on_loaded callback is set, comp.load() will block until the component is loaded
# otherwise, comp.load() will return immediately
comp.load('TopChopDatIO.tox') 

# do stuff here before waiting for the component to be loaded
print('waiting for component to be loaded...')

# wait for the component to be loaded (blocks until the future is set)
future_load.result()

print('component loaded')

comp.start()
comp.unload()






