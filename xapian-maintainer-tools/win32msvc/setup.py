# Distutils script for building packaged Xapian python bindings for Win32

from distutils.core import setup
import xapian
import platform

description = ("""
Win32 MSVC build of Python %s Bindings for the Xapian Open Source Search Engine Library (http://www.xapian.org/)
""" % platform.python_version())

setup(
    name = "xapian-python-bindings",
    version = xapian.xapian_version_string(),
    url = 'http://www.lemurconsulting.com',
    maintainer = 'Charlie Hull (with thanks to Alexandre Gauthier)',
    maintainer_email = 'info@lemurconsulting.com',
    description = description.strip(),
    py_modules = ['xapian'],
    packages = ['xapian'],
    package_dir = {'xapian': 'xapian'},
    package_data = {'xapian': ['_xapian.pyd', 'zlib1.dll']},
)
