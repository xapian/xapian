# Distutils script for building packaged Xapian python bindings for Win32

from distutils.core import setup
from distutils.command.install import INSTALL_SCHEMES
import xapian
import platform
import glob

description = ("""
Win32 MSVC build of Python %s Bindings for the Xapian Open Source Search Engine Library (http://www.xapian.org/)
""" % platform.python_version())

for scheme in INSTALL_SCHEMES.values():
    scheme['data'] = scheme['purelib'] 

setup(
    name = "xapian-python-bindings for Python %s " % platform.python_version(),
    version = xapian.version_string(),
    url = 'http://www.lemurconsulting.com',
    maintainer = 'Charlie Hull (with thanks to Alexandre Gauthier)',
    maintainer_email = 'info@lemurconsulting.com',
    description = description.strip(),
    py_modules = ['xapian'],
    packages = ['xapian'],
    package_dir = {'xapian': 'xapian'},
    package_data = {'xapian': ['_xapian.pyd', 'zlib1.dll','docs/index.html','docs/examples/*.*']},
                   

)
