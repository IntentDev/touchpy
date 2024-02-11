import sys
import os

# get the path to ../out/build/x64-release
path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'out', 'build', 'x64-release'))

# add the path to the python path
sys.path.append(path)
import tdpy

if __name__ == '__main__':

	tdpy.init_vk()

	