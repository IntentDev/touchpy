import sys
from setuptools import setup, find_packages
from wheel.bdist_wheel import bdist_wheel as _bdist_wheel

import warnings
warnings.filterwarnings("ignore", message="Config variable 'Py_DEBUG' is unset")

# Subclass bdist_wheel to customize the wheel filename
class bdist_wheel(_bdist_wheel):
    def finalize_options(self):
        super().finalize_options()
        self.root_is_pure = False 

setup(
    packages=find_packages(),
    package_data={'touchpy': ['*.pyd', '*.dll']},
    zip_safe=False,
    cmdclass={'bdist_wheel': bdist_wheel}  # Use our custom command
)
