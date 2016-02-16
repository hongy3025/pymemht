from distutils.core import setup
from distutils.extension import Extension

try:
    from Cython.Build import cythonize
    ext_modules = cythonize([Extension(
        'memht',
        sources=['memht.pyx', 'hashtable.cpp'],
        extra_compile_args=['-Wno-unused-function', '-Wno-unneeded-internal-declaration']
    )])
except ImportError:
    ext_modules = [Extension(
        'memht',
        sources=['memht.cpp'],
        extra_compile_args=['-Wno-unused-function', '-Wno-unneeded-internal-declaration']
    )]

setup(version='0.1',
      name='memht',
      ext_modules=ext_modules,
      packages=['memht'],
      author='HongYing',
      author_email='hongy3025@163.com',
      description='inplace memory hashtable')
