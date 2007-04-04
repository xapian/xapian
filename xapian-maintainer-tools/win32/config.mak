# Makefile configuration for Microsoft Visual C++ 7.0 (or compatible)
# Thanks to Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
#
# Modify this file to set the Python configuration and any extra Xapian build flags
# Note that you should only use a Windows Python built using Visual C++, i.e. the standard Windows
# binary distribution

# -------------Python settings-------------

# Python folder
PYTHON_DIR=c:\Program Files\Python25
# Python executable
PYTHON_EXE=$(PYTHON_DIR)\python.exe 
#PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE=$(PYTHON_DIR)\include
# PYTHON_LIB : Set this to the python library including path for linking with
PYTHON_LIB=$(PYTHON_DIR)\libs\python25.lib
# Modern or olde: Define exactly the one you want and leave the other one 
# uncommented out
PYTHON_MODERN_OR_OLDE = modern
#PYTHON_MODERN_OR_OLDE = olde
# -------------end Python settings-------------

# -------------PHP settings-------------

# PHP source folder
PHP_SRC_DIR=\work\php-4.4.6
# PHP executable folder
PHP_EXE_DIR=\work\php-4.4.6-win32
# for debug, set the above to:
#PHP_EXE_DIR=\work\php-4.4.6\Debug_TS

PHP_INCLUDE_CPPFLAGS= \
/I "$(PHP_SRC_DIR)" /I "$(PHP_SRC_DIR)\tsrm" /I "$(PHP_SRC_DIR)\Zend" /I "$(PHP_SRC_DIR)\main" /I "$(PHP_SRC_DIR)\regex"  \
/D ZTS=1 /D ZEND_WIN32=1 /D PHP_WIN32=1 /D ZEND_WIN32_FORCE_INLINE /D HAVE_WIN32STD=1 \

# version 4 or 5: Define exactly the one you want and leave the other one 
# commented out
PHP_MAJOR_VERSION = 4
#PHP_MAJOR_VERSION = 5

#Release build
#     PHP_LIB : Set this to the PHP library including path for linking with
PHP_LIB=$(PHP_EXE_DIR)\php4ts.lib
# for debug, set the above to:
#PHP_LIB=$(PHP_EXE_DIR)\php4ts_debug.lib

#    PHP flag for compiling debug/release versions
PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=0"
# for debug, set the above to:
#PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=1"

#    PHP executable
PHP_EXE=$(PHP_EXE_DIR)\PHP.exe 
# ------------- end PHP settings-------------

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
XAPIAN_APPLICATIONS=..\..\omega-0.9.10
XAPIAN_BINDINGS=..\..\xapian-bindings-0.9.10

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

MANIFEST=mt.exe /manifest

