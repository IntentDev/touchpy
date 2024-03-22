import touchpy as tp
import numpy as np


def on_frame1(comp, info):
	print('on_frame1 callback called!')
	info['frame'] += 1
	return

def on_frame2(comp, info):
	print('on_frame2 callback called!')
	info['frame'] += 1
	return

info = {'frame': 0}

comp = tp.Comp('FreeRunningIO.tox')
# comp.set_on_frame_callback(on_frame1, info)
