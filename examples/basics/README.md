# TouchPy Basic Examples

This folder contains basic examples of how to use TouchPy, with relatively thorough explanations of the concepts and tools. Each example focuses on a particular aspect of interfacing with the library. 

## Dependencies
Currently only two examples have dependencies other than TouchPy, 02_chops.py (NumPy) and 06_tops.py (PyTorch). 

	pip install numpy

Follow instructions from https://pytorch.org/ to install PyTorch for your system. Typically something like:

	pip3 install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118

## Usage
- Clone the TouchPy repository:

		git clone https://github.com/IntentDev/touchpy.git
		cd touchpy/examples/basics

- Open ExampleComps.toe to review the component that will be loaded in each example (TopChopDatIO.tox). 
- Close ExampleComps.toe 
- Open ExampleReceive.toe to monitor data being set on Pars and In operators of the component. (If running a TD build that supports opening a window in a TouchEngine instance this is not required).
- Open touchpy/examples/basics folder in a text editor to review code and comments
- To start the first example run:

		python 01_load_comp_basic.py


## Jurigged
Installing Jurigged is also useful for experimenation providing the ability to update code while it's running.

	pip install jurigged

Then run in terminal:

	jurigged 03_set_pars.py

Or any other example script. Now you can edit values, add and remove lines while the script is running and it will update. 