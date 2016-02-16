from distutils.core import setup
from distutils.extension import Extension

ext_name = 'memht'
# extra_compile_args = ['-Wno-unused-function', '-Wno-unneeded-internal-declaration']
extra_compile_args = []

try:
    from Cython.Build import cythonize
    ext_modules = cythonize([Extension(
        ext_name,
        sources=['memht.pyx', 'hashtable.cpp'],
        extra_compile_args=extra_compile_args
    )])
except ImportError:
    ext_modules = [Extension(
        ext_name,
        sources=['memht.cpp', 'hashtable.cpp'],
        extra_compile_args=extra_compile_args
    )]

setup(version='0.1',
      name='memht',
      ext_modules=ext_modules,
      packages=['memht'],
      author='HongYing',
      author_email='hongy3025@163.com',
      description='inplace memory hashtable')
