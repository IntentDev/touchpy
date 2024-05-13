Build Release Instructions
=========================================

Create Conda/Mamba environments for Python 3.9, 3.10, 3.11, 3.12
```bash
conda create -n py39 python=3.9
conda create -n py310 python=3.10
conda create -n py311 python=3.11
conda create -n py312 python=3.12
```

Make sure paths to Python 3.9, 3.10, 3.11 and 3.12 are all in PATH (preferred Python version - 3.11 should be located before the others so it still gets run when using python command in the command prompt)


### To build the modules and documentationn (builds to install directory):
- Open the Visual Studio Developer Command Prompt and run:
```bash
# build compile all configs and copy to install
build_release.bat

# additionally copy all file specified directory
build_release.bat path/to/release/directory

# additionally build package (TOUCHPY_BUILD_ENVS environment variable needs to be set to Python envs location)
# each Python environment needs build installed (pip install build)
build_release.bat package

# additionally build package and upload to PyPI (need to manually set new version # in package/pyproject.toml)
build_release.bat package upload
```
