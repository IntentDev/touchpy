import touchpy as tp
import modules.utils as utils # utils.py is in the same directory as this script

# set the logging level to INFO which will print out verbose information
tp.init_logging(level=tp.LogLevel.INFO)

# create a class that inherits from tp.Comp
class MyComp (tp.Comp):

	# these are the default arguments for tp.Comp
	# flags: are the flags that control the behavior of the component (can only be set at initialization)
	# fps: is the frames per second that the component will run at (can only be set at initialization)
	# device: is the device (GPU) that the component will run on (0 is defualt, can only be set at initialization)
	# td_path: is the path to the TouchDesigner executable ("" will use the latest installed version)
	def __init__(self, flags=tp.CompFlags.INTERNAL_TIME_AUTO, fps=60, device=0, td_path=""):
		
		# call the parent class constructor to initialize the component
		super().__init__(flags=flags, fps=fps, device=device, td_path=td_path)

		# create a frame counter
		self.frame = 0

		# set the on_layout_change_callback to the on_layout_change method
		self.set_on_layout_change_callback(self.on_layout_change, {})

		# set the on_frame_callback to the on_frame method
		self.set_on_frame_callback(self.on_frame, {})

	# this gets called at least once after the component is loaded, at this point
	# all the pars, in and out ops are available. If the a par, in or out op is added or removed
	# this method will be called again
	def on_layout_change(self, info):
		print('layout changed:')
		print('in tops:\n', *[f"\t{name}\n" for name in self.in_tops.names])
		print('out tops:\n', *[f"\t{name}\n" for name in self.out_tops.names])
		print('in chops:\n', *[f"\t{name}\n" for name in self.in_chops.names])
		print('out chops:\n', *[f"\t{name}\n" for name in self.out_chops.names])
		print('in dats:\n', *[f"\t{name}\n" for name in self.in_dats.names])
		print('out dats:\n', *[f"\t{name}\n" for name in self.out_dats.names])
		print('pars:\n', *[f"\t{name}\n" for name in self.par.names])

	# define the on_frame method that will be called on every frame
	# the info argument is user data that can be passed to the callback
	# in this case it is an empty dictionary
	def on_frame(self, info):

		# stop running the if the 'q' key is pressed (terminal must be in focus)
		# always stop the component before calling start_next_frame()
		if utils.check_key('q'):
			self.stop()
			return

		# start_next_frame() is called to advance the component to the next frame
		self.start_next_frame()
		self.frame += 1

# INTERNAL_TIME_AUTO will automatically run the component from this thread
# component will run in a different process but this thread will still block
# when comp.start() is called, after which callbacks are used to do work
flags = tp.CompFlags.INTERNAL_TIME_AUTO

# create an instance of the MyCompBasic class with specific flags, device and td_path
# set td_path to the path of a TouchDesigner installation folder on your system
# if the installation is not found, the latest installed version will be used
td_path = 'C:/Program Files/Derivative/TouchDesigner.2023.11764.22'

fps = 60 
device = 0 

comp = MyComp(flags, fps, device, td_path)

# load the tox file into the component
comp.load('TopChopDatIO.tox') 

# start the component, this will block until self.stop() is called when not running
# the component in async mode
comp.start()

# unload the component to cleanly free up resources
comp.unload()






