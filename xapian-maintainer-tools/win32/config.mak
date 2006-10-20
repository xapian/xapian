# Makefile configuration for Microsoft Visual C++ 7.0 (or compatible)
# Thanks to Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006
#
# Modify this file to set the Python configuration and any extra Xapian build flags
# Note that you should only use a Windows Python built using Visual C++, i.e. the standard Windows
# binary distribution
# ----------------------------------------------
# SWIG Python support
# ----------------------------------------------

# uncomment to enable Python
SWIG_PYTHON=yes

#PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE=c:\python24\include

# PYTHON_LIB : Set this to the python library including path for linking with
PYTHON_LIB=c:\python24\libs\python24.lib

# Modern or olde: Define exactly the one you want and leave the other one 
# uncommented out
PYTHON_MODERN_OR_OLDE = modern
#PYTHON_MODERN_OR_OLDE = olde


# ----------------------------------------------
# Xapian build definitions
# ----------------------------------------------


CPPFLAGS_EXTRA=/I..\include /I..\common
LIBFLAGS=

