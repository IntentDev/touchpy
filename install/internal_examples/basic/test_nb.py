import keyboard
import numpy as np
import torch
import concurrent.futures
import asyncio
import time


import touchpy as tp

tp.init_logging(level=tp.LogLevel.DEBUG)

comp = tp.Comp()
comp = tp.Comp(flags=tp.CompFlagBits.INTERNAL_TIME_AUTO | tp.CompFlagBits.CUDA_STREAM_DEFAULT, device=0)


del comp




