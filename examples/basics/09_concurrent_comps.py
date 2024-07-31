import touchpy as tp
import modules.utils as utils
import asyncio

class MyComp (tp.Comp):
	def __init__(self, name, flags, device=0):
		super().__init__(flags=flags, device=device)

		self.name = name
		self.frame = 0
		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

	def on_layout_change(self, info):
		print('layout changed:')

		if 'Openwindow' in self.par.names:
			self.par['Openwindow'].pulse()

	def on_frame(self, info):
		self.start_next_frame()

		if self.frame % 180 == 0:
			print(f'{self.name} - frame: {self.frame}')

		self.frame += 1

# global dictionary to store the futures for each comp instance
# used to signal when the comp is loaded, stopped, or unloaded
comp_futures = {}

# global loop variable to call futures from the main thread
loop = None

# define the on_loaded callback that will be called when the component is loaded
def on_loaded(info):
	global comp_futures, loop
	comp_id = info['comp_id']
	if comp_id in comp_futures:
		loop.call_soon_threadsafe(comp_futures[comp_id].set_result, True)

# define the on_stop callback that will be called when the component is stopped
def on_stop(info):
	global comp_futures, loop
	comp_id = info['comp_id']
	if comp_id in comp_futures:
		loop.call_soon_threadsafe(comp_futures[comp_id].set_result, True)

# define the on_unloaded callback that will be called when the component is unloaded
def on_unloaded(info):
	global comp_futures, loop
	comp_id = info['comp_id']
	if comp_id in comp_futures:
		loop.call_soon_threadsafe(comp_futures[comp_id].set_result, True)

# async function to load multiple comps concurrently
async def load_comps(comps):
	global comp_futures, loop
	loop = asyncio.get_running_loop()
	
	for comp in comps:
		comp_id = id(comp)
		comp_futures[id(comp)] = loop.create_future()
		comp.set_on_loaded_callback(on_loaded, {'comp_id': comp_id})
		comp.set_on_stop_callback(on_stop, {'comp_id': comp_id})
		comp.set_on_unloaded_callback(on_unloaded, {'comp_id': comp_id})
		comp.load('TopChopDatIO.tox')

	await asyncio.gather(*comp_futures.values())

# async function to start multiple comps concurrently
async def start_comps(comps):
	for comp in comps:
		comp.start()

# async function to wait for the user to press 'q' to continue
async def wait_for_q_key(comps):
	while True:
		if utils.check_key('q'):
			break
		await asyncio.sleep(0.1)

# async function to stop multiple comps concurrently
async def stop_comps(comps):
	global comp_futures, loop
	loop = asyncio.get_running_loop()

	for comp in comps:
		comp_futures[id(comp)] = loop.create_future()
		comp.stop()

	await asyncio.gather(*comp_futures.values())

# async function to unload multiple comps concurrently
async def unload_comps(comps):
	global comp_futures, loop
	loop = asyncio.get_running_loop()

	for comp in comps:
		comp_futures[id(comp)] = loop.create_future()
		comp.unload()

	await asyncio.gather(*comp_futures.values())

# main function to load, start, stop, and unload multiple comps concurrently
async def main():

	base_name = 'my_comp_gpu0_'
	comps = [MyComp(f"my_comp_gpu0_{i}", flags=tp.CompFlags.INTERNAL_TIME_ASYNC) for i in range(3)]

	# uncomment to load 3 comps on the second GPU as well (if available)
	# comps += [MyComp(f"my_comp_gpu1_{i}", flags=tp.CompFlags.INTERNAL_TIME_ASYNC, device=1) for _ in range(3)]

	print('loading comps...')
	await load_comps(comps)

	print('starting comps...')
	await start_comps(comps)

	print('waiting for user to press "q" to continue...')
	await wait_for_q_key(comps)

	print('stopping comps...')
	await stop_comps(comps)

	print('unloading comps...')
	await unload_comps(comps)

asyncio.run(main())









