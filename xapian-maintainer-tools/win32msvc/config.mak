# Makefile configuration for Microsoft Visual C++ 7.0 (or compatible)
# Thanks to Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
#
# Modify this file to set the Python configuration and any extra Xapian build flags
# Note that you should only use a Windows Python built using Visual C++, i.e. the standard Windows
# binary distribution

# -------------Python settings-------------
# uncomment to enable Python
SWIG_PYTHON=yes

# Python folder
PYTHON_DIR=c:\Program Files\Python24
# Python executable
PYTHON_EXE=$(PYTHON_DIR)\python.exe 
#PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE=$(PYTHON_DIR)\include
# PYTHON_LIB : Set this to the python library including path for linking with
PYTHON_LIB=$(PYTHON_DIR)\libs\python24.lib
# Modern or olde: Define exactly the one you want and leave the other one 
# uncommented out
PYTHON_MODERN_OR_OLDE = modern
#PYTHON_MODERN_OR_OLDE = olde
# -------------end Python settings-------------

# Swig executable
SWIG=\work\tools\swigwin-1.3.31\swig.exe

# ----------------------------------------------
# Xapian build definitions
# ----------------------------------------------

# EDIT THESE to match where your applications and bindings are, if you are compiling them
# Also edit:
# win32_applications_omega.mak
# win32_bindings_python.mak
# and any other bindings mak files
XAPIAN_APPLICATIONS=..\..\xapian-applications
XAPIAN_BINDINGS=..\..\xapian-bindings

# We build with the following compiler options:
# /W3 Set warning level to 3
# /O2 Optimisations:Maximise speed
# /EHc extern "C" defaults to nothrow
# /EHs enable C++ EH (no SEH exceptions)
# /c compile, don't link
# /MD Link multithreaded dynamic

# Release build
CPPFLAGS_EXTRA=/I.. /I..\include /I..\common /W3 /EHsc /O2 /MD /c /D "NDEBUG" \
/D "WIN32" /D "__WIN32__" /D "_WINDOWS" \
/D "HAVE_VSNPRINTF" /D "HAVE_STRDUP" /D "_USE_32BIT_TIME_T" \
/D_CRT_SECURE_NO_DEPRECATE
XAPIAN_DEBUG_OR_RELEASE=Release

# Debug build
# CPPFLAGS_EXTRA=/I.. /I..\include /I..\common /W3 /EHsc /Ox /MDd /c /D "_DEBUG" \
#/D "WIN32" /D "__WIN32__" /D "_WINDOWS" \
#/D "HAVE_VSNPRINTF" /D "HAVE_STRDUP" /D "_USE_32BIT_TIME_T" \
#/D_CRT_SECURE_NO_DEPRECATE
#XAPIAN_DEBUG_OR_RELEASE=Debug

LIBFLAGS_EXTRA=
LINKFLAGS_EXTRA=

