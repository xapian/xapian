from setuptools import setup, Extension
from pathlib import Path

version = '1.5.0+master'
xapian_core = Path('../../xapian-core')

setup(version=version,
      ext_modules=[Extension(name='xapian._xapian',
                             sources=['xapian_wrap.cc'],
                             library_dirs=[str(xapian_core / '.libs')],
                             libraries=['xapian-1.5'],
                             include_dirs=[str(xapian_core),
                                           str(xapian_core / 'include')])])


