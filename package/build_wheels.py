import subprocess
import shutil
import os


required_python_folders = ["py39", "py310", "py311", "py312"]
python_paths = {}

def set_python_paths():
    # search through PATH entries for suitable python executable
    for path in os.environ["PATH"].split(";"):
        # remove last forward or backslash if present
        path = path.rstrip("\\").rstrip("/")
        last_folder = os.path.basename(path)
        if last_folder.startswith("py"):
            python_paths[last_folder] = os.path.join(path, "python.exe")

    # check if all required python versions are found
    for folder in required_python_folders:
        if folder not in python_paths:
            msg = f"Could not find Python installation named: {folder}"
            raise FileNotFoundError(msg)

    

def build_wheel(python_version, pyd_file, output_dir="dist"):
    """Builds a wheel for the given Python version."""
    
    py_folder = f"py{python_version}"
    python_executable = python_paths.get(py_folder, None)

    # Copy the appropriate .pyd file to the touchpy folder
    pyd_filename = os.path.basename(pyd_file)
    shutil.copy(pyd_file, os.path.join("touchpy", pyd_filename))

    # Build the wheel
    subprocess.run(
        [python_executable, "-m", "build", "--wheel", "--outdir", output_dir],
        check=True,  
        cwd=os.getcwd()  
    )

    # Remove the copied .pyd file
    os.remove(os.path.join("touchpy", pyd_filename))

if __name__ == "__main__":

    # set __version__ in touchpy/__init__.py to the version specified in pyproject.toml
    version = None
    with open("pyproject.toml") as f:
        for line in f:
            if "version" in line:
                version = line.split("=")[1].strip().strip('"')
                break

    if version is None:
        raise ValueError("Could not find version in pyproject.toml")
    
    with open("touchpy/__init__.py") as f:
        lines = f.readlines()

    with open("touchpy/__init__.py", "w") as f:
        for line in lines:
            if "__version__" in line:
                f.write(f'__version__ = "{version}"\n')
            else:
                f.write(line)

    python_versions = ["39", "310", "311", "312"]
    pyd_files = [
        "pyd_files\\touchpy.cp39-win_amd64.pyd",
        "pyd_files\\touchpy.cp310-win_amd64.pyd",
        "pyd_files\\touchpy.cp311-win_amd64.pyd",
        "pyd_files\\touchpy.cp312-win_amd64.pyd"
    ]

    for version, pyd in zip(python_versions, pyd_files):
        set_python_paths()
        build_wheel(version, pyd)
