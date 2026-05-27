# TouchPy

TouchPy is a high-performance Python toolset for working with TouchDesigner components headlessly via the TouchEngine SDK. It provides GPU-accelerated data exchange (CUDA/Vulkan) for TOPs, CHOPs, DATs, and parameters.

## Requirements

- Windows 10/11 (x64)
- [TouchDesigner](https://derivative.ca/download) installed (runtime dependency)
- NVIDIA GPU with CUDA support
- Python 3.9 - 3.13

## Installation

If you simply want to use TouchPy and run the examples, you don't need to clone this repo:

```bash
uv add touchpy[examples]
```

This installs TouchPy with CUDA-enabled PyTorch and numpy for running the examples.

> [uv](https://docs.astral.sh/uv/) is required for dependency management. Install it via `pip install uv` or see the [uv docs](https://docs.astral.sh/uv/getting-started/installation/).

## Quick Start

```python
import touchpy as tp

comp = tp.Comp()
comp.load("path/to/component.tox")
comp.start()

if comp.frame_did_finish():
    comp.start_next_frame()
```

See the `examples/` directory for more detailed usage including TOPs with PyTorch tensors, CHOPs, DATs, and parameter control.

## Documentation

Full API documentation is available at [intentdev.github.io/touchpy](https://intentdev.github.io/touchpy/).

---

## Development

### Prerequisites

- [Visual Studio 2022](https://visualstudio.microsoft.com/) with C++ desktop workload
- [CMake](https://cmake.org/) 3.21+
- [CUDA Toolkit](https://developer.nvidia.com/cuda-toolkit) 12.x
- [Vulkan SDK](https://vulkan.lunarg.com/)
- [uv](https://docs.astral.sh/uv/) for Python dependency management

### Setup

```bash
uv venv --python 3.12
uv sync --extra examples
```

This creates a `.venv` with Python 3.12, builds TouchPy from source (via scikit-build-core), and installs CUDA-enabled PyTorch + numpy for running examples.

> **Note:** Python 3.12 is recommended. PyTorch wheels for 3.13+ may have packaging issues.

### Rebuilding after C++ changes

```bash
uv sync --extra examples --reinstall-package touchpy
```

Or equivalently:

```bash
uv pip install -ve .
```

### Build a wheel

```bash
uv build
```

### Verify

```python
import touchpy
print(touchpy.__version__)  # shows 0.12.0.dev0 for local dev builds
```
