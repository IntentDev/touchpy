[![PyPI version](https://badge.fury.io/py/touchpy.svg)](https://badge.fury.io/py/touchpy)
[![PyPI - Python Version](https://img.shields.io/pypi/pyversions/touchpy)](https://pypi.org/project/touchpy/)



# TouchPy
TouchPy is a high-performance toolset to work with TouchDesigner components in Python.

By leveraging Vulkan, CUDA, and TouchEngine, TouchPy opens new pathways for integration, particularly with libraries such as PyTorch and Nvidia Warp. TouchPy supports GPU-to-GPU (zero-copy) data transfers, streamlining data exchange between standalone Python applications and Touchdesigner.

Please refer to the project [Documentation](https://intentdev.github.io/touchpy/) for installation instructions, API and language reference.

More usage examples and tutorials are coming soon. 

#### Video introduction / tutorials:
- The first public version of TouchPy was released during the TouchDesigner Event Berlin 2024 - see the [video of the TouchPy release presentation](https://www.youtube.com/live/hxCsPlc6W-o?t=10315s).

- The TouchPy workshop on Friday May 24th at Spatial Media Lab in Berlin was also recorded, here's the [video of the TouchPy workshop at SML](https://www.youtube.com/watch?v=XDZkcEkWTOE).

## Installation

TouchPy supports Python 3.9 onwards and runs on Windows.
A Vulkan capable GPU and driver is required. To work with TOPs a Nvidia card is required.

As TouchPy uses TouchEngine, it requires TouchDesigner or TouchPlayer (release 2023 or later) to be installed with a paid license (Educational, Commercial, or Pro).

The easiest way to install TouchPy is:

$ pip install touchpy

