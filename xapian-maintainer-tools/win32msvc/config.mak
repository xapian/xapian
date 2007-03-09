# Makefile configuration for Microsoft Visual C++ 7.0 (or compatible)
# Thanks to Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
#
# Modify this file to set the Python configuration and any extra Xapian build flags
# Note that you should only use a Windows Python built using Visual C++, i.e. the standard Windows
# binary distribution

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
# ----------------------------------------------
# Xapian paths
# ----------------------------------------------

# EDIT THESE to match where your applications and bindings are, if you are compiling them
# Also edit:
# win32_applications_omega.mak
# win32_bindings_python.mak
# and any other bindings mak files
XAPIAN_APPLICATIONS=..\..\xapian-applications
XAPIAN_BINDINGS=..\..\xapian-bindings

#  ------------- Perl settings-------------
# Perl folder
PERL_DIR=C:\work\Perl\bin
# Perl executable
PERL_EXE=$(PERL_DIR)\perl.exe
# -------------end Perl settings-------------


# -------------Python settings-------------
# Python folder
PYTHON_DIR=c:\Program Files\Python25
# Python executable
PYTHON_EXE=$(PYTHON_DIR)\python.exe 
#PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE=$(PYTHON_DIR)\include
# PYTHON_LIB : Set this to the python library including path for linking with
PYTHON_LIB=$(PYTHON_DIR)\libs\python25.lib
# -------------end Python settings-------------


# -------------PHP settings-------------
# PHP folder
PHP_DIR=\work\php-4.4.6-Win32
#PHP_INCLUDE : Set this to the directory that contains PHP.h
PHP_INCLUDE=\work\php-4.4.6
PHP_INCLUDE_CPPFLAGS= \
/I "$(PHP_INCLUDE)" /I "$(PHP_INCLUDE)\tsrm" /I "$(PHP_INCLUDE)\Zend" /I "$(PHP_INCLUDE)\main" /I "$(PHP_INCLUDE)\regex"  \
/D"HAVE_WIN32STD=1" /D "ZEND_WIN32" /D "PHP_WIN32" /D ZEND_WIN32_FORCE_INLINE /D ZTS 

# version 4 or 5: Define exactly the one you want and leave the other one 
# uncommented out
#PHP_MAJOR_VERSION = 5
PHP_MAJOR_VERSION = 4

#Release build
#     PHP_LIB : Set this to the PHP library including path for linking with
#PHP_LIB=$(PHP_DIR)\Release_TS\php-5.2.1\dev\php5ts.lib
PHP_LIB=$(PHP_DIR)\php4ts.lib
#    PHP flag for compiling debug versions
PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=0"
#    PHP executable
#PHP_EXE=$(PHP_DIR)\Release_TS\php-5.2.1\PHP.exe 
PHP_EXE=$(PHP_DIR)\PHP.exe 

# Debug build
#     PHP_LIB : Set this to the PHP library including path for linking with
#PHP_LIB=$(PHP_DIR)\Debug_TS\php5ts_debug.lib
#    PHP executable
#PHP_EXE=$(PHP_DIR)\Debug_TS\PHP.exe 
#    PHP flag for compiling debug versions
#PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=1"
# ------------- end PHP settings-------------


# ------------SWIG settings-------------
# Swig executable
SWIG=\work\tools\swigwin-1.3.31\swig.exe
SWIG_FLAGS= -Werror -noproxy
# ------------end SWIG settings-------------


#--------------------------------------
# Visual C++ Compiler and linker programs, and flags for these
#--------------------------------------
LIB32=link.exe -lib
LIB32_FLAGS=/nologo  
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console
CPP=cl.exe
RSC=rc.exe
MANIFEST=mt.exe /manifest

# We build with the following compiler options:
# /W3 Set warning level to 3
# /O2 Optimisations:Maximise speed
# /EHc extern "C" defaults to nothrow
# /EHs enable C++ EH (no SEH exceptions)
# /c compile, don't link
# /MD Link multithreaded dynamic libraries

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

#----------------end Visual C++----------------------

