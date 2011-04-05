from distutils.core import setup, Extension

tiger = Extension('tiger', sources = ['sboxes.c', 'tiger.c', 'pytiger.c',
                                       'tigertree.c'])

setup(name = 'Tiger Hash',
      version = '1.0',
      author = 'Mark Lee',
      description = 'Implementation of the Tiger Hash for Python',
      ext_modules = [tiger])
