# Makefile configuration for Microsoft Visual C++ 7.0 (or compatible)
# Thanks to Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
#
# Modify this file to set any extra Xapian build flags
#
# HINT: Instead of modifying this file, consider passing new values
# on the command-line.  For example:
#  % nmake  PERL_DIR=c:\perl\bin SWIG=c:\something\swig.exe
# would override the variables without requiring you change anything...

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
# Note that you should only use a Windows Python built using Visual C++, i.e. the standard Windows
# binary distribution

# Python folder
PYTHON_DIR=c:\Program Files\Python25
# Python executable
PYTHON_EXE=$(PYTHON_DIR)\python.exe 
 #PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE=$(PYTHON_DIR)\include
#A 'PC' directory is also included for people building from a source tree.
PYTHON_INCLUDE_2=$(PYTHON_DIR)\PC

# PYTHON_LIB_DIR : Set this to the directory containing python*.lib
# It should only be necessary to change this for source builds of Python,
# where the files are in 'PCBuild' rather than 'libs' (this magically works
# as Python uses a #pragma to reference the library base name - which
# includes any version numbers and debug suffixes ('_d'))
PYTHON_LIB_DIR=$(PYTHON_DIR)\libs
# -------------end Python settings-------------


# -------------PHP settings-------------
# PHP source folder
PHP_SRC_DIR=\work\php-5.2.1

PHP_INCLUDE_CPPFLAGS= \
/I "$(PHP_SRC_DIR)" /I "$(PHP_SRC_DIR)\tsrm" /I "$(PHP_SRC_DIR)\Zend" /I "$(PHP_SRC_DIR)\main" /I "$(PHP_SRC_DIR)\regex"  \
/D ZTS=1 /D ZEND_WIN32=1 /D PHP_WIN32=1 /D ZEND_WIN32_FORCE_INLINE /D HAVE_WIN32STD=1 \

# version 4 or 5: Define exactly the one you want and leave the other one 
# commented out. Note you will have to modify the paths below as well.
#PHP_MAJOR_VERSION = 4
PHP_MAJOR_VERSION = 5

# PHP_EXE_DIR: Set this to the folder where the PHP executable is
# PHP_LIB : Set this to the path to the PHP library 
!if "$(DEBUG)"=="1"
PHP_EXE_DIR=\work\php-5.2.1\Debug_TS
PHP_LIB=$(PHP_EXE_DIR)\php5ts_debug.lib
PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=1"
!else
PHP_EXE_DIR=\work\php-5.2.1-win32
PHP_LIB=$(PHP_EXE_DIR)\dev\php5ts.lib
PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=0"
!endif

#    PHP executable
PHP_EXE=$(PHP_EXE_DIR)\PHP.exe 
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
 wsock32.lib Ws2_32.lib  odbccp32.lib /subsystem:console /debug /nologo
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

# Common stuff
# Note we enable debug flags for a release build - this means that
# even in release builds, a .pdb file is generated with basic
# stackframe information, meaning basic debugging on release builds
# is still possible (so long as the .pdb files are in place - it is
# assumed these files will *not* ship with a default binary build)
CPPFLAGS_COMMON=/nologo /c /Zi /I.. /I..\include /I..\common /W3 /EHsc \
/D "WIN32" /D "__WIN32__" /D "_WINDOWS" \
/D "HAVE_VSNPRINTF" /D "HAVE_STRDUP" /D "_USE_32BIT_TIME_T" \
/D_CRT_SECURE_NO_DEPRECATE


!IF "$(DEBUG)" == "1"
# Debug build
CPPFLAGS_EXTRA=$(CPPFLAGS_COMMON) /Od /MDd /D DEBUG /D _DEBUG /D XAPIAN_DEBUG
XAPIAN_DEBUG_OR_RELEASE=Debug
!ELSE
# Release build
CPPFLAGS_EXTRA=$(CPPFLAGS_COMMON) /O2 /MD /D NDEBUG
XAPIAN_DEBUG_OR_RELEASE=Release
!ENDIF

#----------------end Visual C++----------------------

