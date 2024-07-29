import touchpy as tp

# create a class that inherits from tp.Comp
# inheriting from tp.Comp is not required but it is the recommended way to interface
# with a component

class MyComp (tp.Comp):
	def __init__(self):
		
		# call the parent class constructor to initialize the component
		super().__init__()

		# create a frame counter
		self.frame = 0

		# set the on_frame_callback to the on_frame method
		self.set_on_frame_callback(self.on_frame, {})

	# define the on_frame method that will be called on every frame
	# the info argument is user data that can be passed to the callback
	# in this case it is an empty dictionary
	def on_frame(self, info):

		# stop running the component after 1200 frames (20 seconds at 60 fps)
		if self.frame == 1200:
			self.stop()
			return

		# start_next_frame() is called to advance the component to the next frame
		self.start_next_frame()
		self.frame += 1

# create an instance of the MyCompBasic class
comp = MyComp()

# load the tox file into the component
comp.load('TopChopDatIO.tox') 

# start the component, this will block until self.stop() is called when not running
# the component in async mode
comp.start()

# unload the component to cleanly free up resources
comp.unload()






