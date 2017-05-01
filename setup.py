from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

setup(
	name = "cvsu",
	version = "0.0.1",
	ext_modules = cythonize(
		[Extension("cvsu", ["cvsu.pyx"], 
			libraries=["cvsu"],
			include_dirs=["."])]
	)
)
