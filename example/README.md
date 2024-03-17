# TouchPy
Toolkit providing various interconnectivity tools between TouchDesigner and Python.


### Windows Install procedure

#### 1. Mamba
It’s recommended to install Mamba instead of (Ana)conda. If you have Conda installed, remove it first. 
Mamba is a C++ rewrite of (Python based) conda, can download packages in parallel, and is therefore much faster. The mamba commands are exactly the same as for conda, just replace “conda” with “mamba”.

Download and run this Mamba installer:<br>
https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Windows-x86_64.exe<br>
During install, enable both options "Create start menu shortcuts" and "Add Miniforge3 to my PATH environment variable" 


#### 2. Setup touchpy environment


- start the mamba prompt by searching for “Miniforge prompt” in Windows Startup menu
- in the mamba prompt, cd to this source directory then enter this command to create a new env called touchpy: 

```
mamba env create -f environment.yml
```
(this can take a few minutes to download and install all packages)



#### 3. Binaries
No installation for TouchPy itself is required at this time. The file touchpy.cp311-win_amd64.pyd must be located either in the working directory or in the system PATH. TouchEngine.dll and cudart64_110.dll must be located beside touchpy.cp311-win_amd64.pyd. The simplest way to use TouchPy is to open a command prompt in the directory that contains both the Python file to execute and the TouchPy module.

## Example Usage

```
import touchpy as tp
import torch
import numpy as np

class MyClass:
	def __init__(self):
		self.running = True

	@staticmethod
	def on_frame(comp, this):
		tensor = comp.out_tops[0].as_tensor()
		tensor = tensor * 2
		comp.in_tops[0].from_tensor(tensor)

		arr = comp.out_chops['anOutChopName'].as_numpy()
		arr += 42
		comp.in_chops[0].from_numpy(arr)

		text = comp.out_dats[0].as_string()
		datTable = tp.DatTable()
		datTable.from_list([['a', 'b', 'c'], ['d', 'e', 'f'], [1, 2, 3]], cast=True)

		color_par = comp.par['Acolorparname']
		color_par.set(.2, .4, .8, 1.) 

		comp.start_next_frame():
		# do work on non-comp members for the next frame here
	
	def run_comp(self, path):
		comp = tp.Comp(path)
		comp.set_on_frame_callback(self.on_frame, self)
		comp.start()


myClass = MyClass()
myClass.run_comp("MyComponent.tox")

```

- See example_run_comp.py for more examples


## Comp class
A TouchDesigner component loaded in a TouchEngine instance.

### Methods
- `load_tox(path)`: Loads a .tox file at the specified path.
- `loaded() -> bool`: Returns a boolean indicating whether or not the component is loaded.
- `update(start_next_frame = False)`: Called in a loop repeatedly to update the component.
- `start_next_frame()`: If the `start_next_frame` argument in `update()` is False, this needs to be called after getting and setting all component members.
- `start(update_starts_next_frame=False)`: Starts a loop that calls `update()` repeatedly.
- `stop()`: Stops the loop that calls `update()`.
- `set_on_frame_callback()`: Sets a callback function to be called every frame by the `update()` function when data is ready to be retrieved.
- `in_tops`: InTopLinks container of InTopLink instances.
- `out_tops`: OutTopLinks container of OutTopLink instances.
- `in_chops`: InChopLinks container of InChopLink instances.
- `out_chops`: OutChopLinks container of OutChopLink instances.
- `in_dats`: InDatLinks container of InDatLink instances.
- `out_dats`: OutDatLinks container of OutDatLink instances.
- `par`: ParLinkCollection of ParLink instances.

## Links class (OutTopLinks, InTopLinks, OutChopLinks, InChopLinks, OutDatLinks, InDatLinks classes)
A container of links.

### Methods
- `num_links()`: Returns the number of links in the container.
- `link_names()`: Returns a list of the names of the links in the container.
- `[index]`: Returns the link instance at the given index.
- `[name]`: Returns the link instance with the given name.

## ParLinkCollection class
A collection of parameter links.

### Properties
- `count`: The number of parameter links in the collection.
- `names`: A list of names of all available parameter links.

### Subscript Operator
- `[name]`: Gets a parameter link by name.

## OutTopLink class
An output top link.

### Methods
- `cuda_memory()`: Returns a CudaMemory instance.
- `as_dlpack()`: Returns a Dlpack capsule.
- `as_tensor()`: Returns a PyTorch tensor.

## InTopLink class
An input top link.

### Methods
- `copy_cuda_memory(cudamemory)`: Copies the data from a CudaMemory instance.
- `from_dlpack(capsule)`: Sets the data from a Dlpack capsule (overloads for 1, 2, 3, 4 component arrays).
- `from_tensor(tensor)`: Sets the data from a PyTorch tensor (overloads for 1, 2, 3, 4 component arrays).

## OutChopLink class
An output chop link.

### Methods
- `chan_names()`: Returns a list of the channel names.
- `as_numpy()`: Returns a numpy array (will automatically copy the data if needed).
- `as_numpy_ref()`: Returns a numpy array reference (will not copy the data, unless explicitly copied).

## InChopLink class
An input chop link.

### Methods
- `from_numpy(array, names=[])`: Sets the data from a numpy array (2d), with optional channel names (must be exactly the same length as the size of the first dimension of the array).

## DatTable class
A table in a DAT link

### Methods
- `num_rows`: Returns the number of rows in the table
- `num_cols`: Returns the number of columns in the table
- `row(index)`: Returns a single row as a list of strings
- `col(index)`: Returns a single column as a list of strings
- `cell(row_index, col_index)`: Returns a cell as a string
- `as_list()`: returns a list of lists each of which is a row in the table
- `from_list(list, cast=False)`: set table data from a list of lists (will use first list in parent list for number of columns)

## OutDatLink class
An output dat link.

### Methods
- `as_string()`: Returns the data as a string.
- `as_table()`: Returns the data as a DatTable instance.
- `getTypeDescription()`: Returns the type description of the data (string or DatTable).

## InDatLink class
An input dat link.

### Methods
- `from_string(string)`: Sets the data as text from a string (DAT will be contain text).
- `from_table(table)`: Sets the data as a table from a DatTable instance.
- `from_list(list)`: Sets the data as a table from a list of lists.

## ParLink class
A parameter link.

### Properties
- `val`: Gets or sets the value of the parameter.

### Methods
- `set(val)`: Sets the value of the parameter. Various overloads for multi-component parameters.
- `get()`: Gets the value of the parameter.

## ParLinkValue types
- Int
- Float
- String
- Color
- Float2
- Float3
- Float4
- Int2
- Int3
- Int4
- Bool

## CUDADataType enum class
An enum class representing CUDA data types.

### Values
- Float32
- Int32
- UInt8

## CudaMemoryShape class
A class representing the shape of a CUDA memory.

### Properties
- `width`
- `height`
- `num_components`
- `component_size`
- `data_type` (CUDADataType)
- `strides` (array of 3 ints)

## CudaMemory class
A class representing CUDA memory.

### Properties
- `ptr`: Gets or sets the pointer to the data.
- `size`: Gets or sets the size of the data.
- `shape`: Gets or sets the shape of the data.

## ComponentMask enum class
An enum class representing component masks for tensors.

### Values
- R
- G
- B
- A
- RGB
- RGBA

## TODO / roadmap
- More functionality for In and Out links to get and set data.
	- get single channel from a chopLink
	- set single channel in a chopLink
	- get single row from a datLink
	- set single row in a datLink
	- etc...
- Implement caching mechanism combined with a lazy loading pattern for ChopLinks and DatLinks.
- Create a singleton class for renderer/VkInstance.
- create option to run update loop in thread in order to run jupyter notebooks/interactive sessions
- Add device, time mode and other arguments to Comp constructor
- Expose frame rate and other timing information to Python.
- Set and get time-based FloatBuffers (chops).
- Display and monitor OutTopLink, tensors, etc. in a viewer window.
- Implement various Python tools and operators for sharing memory with TouchDesigner.
	- Python sharedMemIn/Out for CHOPs (SharedMemFromChop, SharedMemToChop)
	- Python SharedMemFromDat, SharedMemToDat
	- TD cplusplusDAT plugins: sharedMemOutDAT, sharedMemInDAT
	- Python (and internal) SharedMemBlob (for arbitrary data types)
	- TD sharedMemIn/out blob (DAT?) (for C++ and Python, blobs)
		- shared blobs will be publicly exposed but will mostly be used internally for calling, functions and
		setting arbitrary datatypes either in TD or from TD - in custom operators or for Python on extensions. 
- Implement TD FromTensorTOP and TD AsTensorTOP.



