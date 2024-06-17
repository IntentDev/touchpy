Build Release Instructions
=========================================

Create Conda/Mamba environments for Python 3.9, 3.10, 3.11, 3.12
```bash
conda create -n py39 python=3.9
conda create -n py310 python=3.10
conda create -n py311 python=3.11
conda create -n py312 python=3.12
```

- Make sure paths to Python 3.9, 3.10, 3.11 and 3.12 are all in PATH (preferred Python version - 3.11 should be located before the others so it still gets run when using python command in the command prompt)
- To build packages each Python environment needs `build` installed: `pip install build`
- When building packages the version number must manually be set in `package/pyproject.toml`

### Build the modules and documentation, package distribution and upload
- Open the Visual Studio Developer Command Prompt and run:
```bash
# build compile all configs and copy to install
build.bat

# additionally build package
build.bat package

# additionally upload to TestPyPI
build.bat package test_upload

# or additionally upload to PyPI
build.bat package upload

# additionally copy all files to specified directory (deprecated?)
build.bat path/to/release/directory

```
