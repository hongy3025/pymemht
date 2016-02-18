from distutils.core import setup
from distutils.extension import Extension
from distutils.command.build_ext import build_ext

extra_compile_args = {
    'msvc': ['/wd4146'],
    'gcc': ['-Wno-unused-function', '-Wno-unneeded-internal-declaration'],
}

extra_libraries = {
    'msvc': ['Ws2_32'],
    'gcc': [],
}

try:
    from Cython.Build import cythonize
    ext_modules = cythonize([
        Extension(
            'memht',
            sources=['memht.pyx', 'hashtable.cpp'],
        ),
        Extension(
            'named_sema',
            sources=['named_sema.pyx']
        ),
    ])
except ImportError:
    ext_modules = [
        Extension(
            'memht',
            sources=['memht.cpp', 'hashtable.cpp'],
        ),
        Extension(
            'named_sema',
            sources=['named_sema.cpp']
        ),
    ]

class BuildExtSubclass(build_ext):
    def build_extensions(self):
        c = self.compiler.compiler_type
        extra_copts = extra_compile_args.get(c)
        if extra_copts is None:
            extra_copts = extra_compile_args.get('gcc')
        extra_libs = extra_libraries.get(c)
        if extra_libs is None:
            extra_libs = extra_libraries.get('gcc')
        for e in self.extensions:
            e.extra_compile_args += extra_copts
            e.libraries += extra_libs
        build_ext.build_extensions(self)

setup(version='0.1',
      name='memht',
      ext_modules=ext_modules,
      packages=['memht'],
      author='HongYing',
      author_email='hongy3025@163.com',
      cmdclass={'build_ext': BuildExtSubclass},
      description='inplace memory hashtable')
