Installation
============

TouchPy supports Python 3.9 onwards and runs on Windows. To work with TOPs a Nvidia card is required.
TouchPy requires TouchDesigner or TouchPlayer to be installed with a paid license (Educational, Commercial, or Pro).

The easiest way to install TouchPy is from `PyPI <https://pypi.org/project/touchpy>`_:

.. code-block:: sh

    $ pip install touchpy

Requirements
------------
TouchPy supports Python versions 3.9 onwards.

The following optional dependencies are required to support certain features:

* `PyTorch <https://pytorch.org/get-started/locally/>`: Required for PyTorch interoperability.

Building the TouchPy documentation requires:

* `Sphinx <https://www.sphinx-doc.org>`_
* `Furo <https://github.com/pradyunsg/furo>`_
* `Sphinx-copybutton <https://sphinx-copybutton.readthedocs.io/en/latest/index.html>`_
* `Sphinx-autoapi <https://sphinx-autoapi.readthedocs.io/en/latest/>`_