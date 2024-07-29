=====================
TouchPy Documentation
=====================

*TouchPy* is a high-performance toolset to work with TouchDesigner components in Python. By leveraging Vulkan, CUDA, and TouchEngine, TouchPy opens new pathways for integration, particularly with libraries such as PyTorch and Nvidia Warp. TouchPy supports GPU-to-GPU (zero-copy) data transfers, streamlining data exchange between standalone Python applications and Touchdesigner.

The first public version of TouchPy was released during the TouchDesigner Event Berlin in May 2024. 


Quickstart
==========

TouchPy supports Python 3.9 onwards and runs on Windows. To work with TOPs a Nvidia card is required.
TouchPy requires TouchDesigner or TouchPlayer to be installed with a paid license (Educational, Commercial, or Pro).

The easiest way to install TouchPy is from `PyPI <https://pypi.org/project/touchpy>`_:

.. code-block:: sh

    $ pip install touchpy


Full Table Of Contents
-----------------------

.. toctree::
   :maxdepth: 1

   self
   install
   overview
   license

.. toctree::
    :maxdepth: 2
    :caption: Core Reference
   
    types
    reference/touchpy/index

.. toctree::
    :maxdepth: 2
    :caption: Examples
   
    examples/basic
