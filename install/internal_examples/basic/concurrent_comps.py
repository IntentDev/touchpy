import keyboard
import numpy as np
import torch
import asyncio

import touchpy as tp

tp.init_logging(level=tp.LogLevel.DEBUG)

comp_futures = {}
loop = None

class MyComp(tp.Comp):
	def __init__(self, flags=tp.CompFlags.INTERNAL_TIME_ASYNC | tp.CompFlags.CUDA_STREAM_DEFAULT, device=0):
		super().__init__(flags=flags, device=device)
		self.device = torch.device(f"cuda:{device}")
		self.frame = 0

		self.set_on_layout_change_callback(self.on_layout_change, {})
		self.set_on_frame_callback(self.on_frame, {})

	def on_layout_change(self, info):
		if 'Openwindow' in self.par.names:
			self.par['Openwindow'].pulse()

	def on_frame(self, info):
		self.start_next_frame()
		self.frame += 1

def on_loaded(info):
	global comp_futures, loop
	comp_id = info['comp_id']
	if comp_id in comp_futures:
		loop.call_soon_threadsafe(comp_futures[comp_id].set_result, True)

def on_stop(info):
	global comp_futures, loop
	comp_id = info['comp_id']
	if comp_id in comp_futures:
		loop.call_soon_threadsafe(comp_futures[comp_id].set_result, True)

async def load_comps(comps):
	global comp_futures, loop
	loop = asyncio.get_running_loop()
	
	for comp in comps:
		comp_id = id(comp)
		comp_futures[id(comp)] = loop.create_future()
		comp.set_on_loaded_callback(on_loaded, {'comp_id': comp_id})
		comp.set_on_stop_callback(on_stop, {'comp_id': comp_id})
		comp.load('TopChopDatIO.tox')

	await asyncio.gather(*comp_futures.values())

async def start_comps(comps):
	for comp in comps:
		comp.start()


async def wait_for_ctrl_q(comps):
	while True:
		if keyboard.is_pressed('ctrl+q'):
			break
		await asyncio.sleep(0.1)

async def stop_comps(comps):
	global comp_futures, loop
	loop = asyncio.get_running_loop()

	for comp in comps:
		comp_futures[id(comp)] = loop.create_future()
		comp.stop()

	await asyncio.gather(*comp_futures.values())

async def unload_comps(comps):
	for comp in comps:
		comp.unload()


async def main():
	comps = [MyComp() for _ in range(3)]

	# load 3 comps on the second GPU as well
	comps += [MyComp(flags=tp.CompFlags.INTERNAL_TIME_ASYNC, device=1) for _ in range(3)]

	await load_comps(comps)
	await start_comps(comps)
	await wait_for_ctrl_q(comps)
	await stop_comps(comps)
	await unload_comps(comps)


if __name__ == '__main__':
	asyncio.run(main())

	print('Test complete.')




