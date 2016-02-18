from distutils.core import setup
from distutils.extension import Extension
from distutils.command.build_ext import build_ext

ext_name = 'memht'

extra_compile_args = {
    'msvc': ['/wd4146'],
    'gcc': ['-Wno-unused-function', '-Wno-unneeded-internal-declaration'],
}

try:
    from Cython.Build import cythonize
    ext_modules = cythonize([Extension(
        ext_name,
        sources=['memht.pyx', 'hashtable.cpp'],
    )])
except ImportError:
    ext_modules = [Extension(
        ext_name,
        sources=['memht.cpp', 'hashtable.cpp'],
    )]

class BuildExtSubclass(build_ext):
    def build_extensions(self):
        c = self.compiler.compiler_type
        extra_copts = extra_compile_args.get(c, 'gcc')
        for e in self.extensions:
            e.extra_compile_args += extra_copts
        build_ext.build_extensions(self)

setup(version='0.1',
      name='memht',
      ext_modules=ext_modules,
      packages=['memht'],
      author='HongYing',
      author_email='hongy3025@163.com',
      cmdclass={'build_ext': BuildExtSubclass},
      description='inplace memory hashtable')
