Build Release Instructions
=========================================

Create Conda/Mamba environments for Python 3.9, 3.10, 3.12 (assuming you already have 3.11)
```bash
conda create -n py39 python=3.9
conda create -n py310 python=3.10
conda create -n py312 python=3.12
```

Make sure paths to Python 3.9, 3.10, 3.11 and 3.12 are all in PATH (preferred Python version - 3.11 should be located before the others so it still gets run when using python command in the command prompt)


### To build the modules and documentationn (builds to install directory):
- Open the Visual Studio Developer Command Prompt and run:
```bash
build_release.bat
```

### To build the modules and documentationn and then copy the files to a specific location:
- Open the Visual Studio Developer Command Prompt and run:
```bash
build_release.bat path/to/release/directory
```


