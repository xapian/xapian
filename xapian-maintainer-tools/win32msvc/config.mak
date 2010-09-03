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
XAPIAN_APPLICATIONS=..\..\xapian-applications\omega
XAPIAN_BINDINGS=..\..\xapian-bindings

#  ------------- Perl settings-------------
# Perl folder
PERL_DIR=C:\Perl\bin
# Perl executable
PERL_EXE=$(PERL_DIR)\perl.exe
# -------------end Perl settings-------------


# -------------Python settings-------------
# Note that you should only use a Windows Python built using Visual C++, i.e. the standard Windows
# binary distribution

# Python folder for 2.4
PYTHON_DIR_24=c:\Python24
# Python executable
PYTHON_EXE_24=$(PYTHON_DIR_24)\python.exe 
 #PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE_24=$(PYTHON_DIR_24)\include
#A 'PC' directory is also included for people building from a source tree.
PYTHON_INCLUDE_2_24=$(PYTHON_DIR_24)\PC

# PYTHON_LIB_DIR : Set this to the directory containing python*.lib
# It should only be necessary to change this for source builds of Python,
# where the files are in 'PCBuild' rather than 'libs' (this magically works
# as Python uses a #pragma to reference the library base name - which
# includes any version numbers and debug suffixes ('_d'))
PYTHON_LIB_DIR_24=$(PYTHON_DIR_24)\libs

# Python folder for 2.5
PYTHON_DIR_25=c:\Python25
# Python executable
PYTHON_EXE_25=$(PYTHON_DIR_25)\python.exe 
 #PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE_25=$(PYTHON_DIR_25)\include
#A 'PC' directory is also included for people building from a source tree.
PYTHON_INCLUDE_2_25=$(PYTHON_DIR_25)\PC

# PYTHON_LIB_DIR : Set this to the directory containing python*.lib
# It should only be necessary to change this for source builds of Python,
# where the files are in 'PCBuild' rather than 'libs' (this magically works
# as Python uses a #pragma to reference the library base name - which
# includes any version numbers and debug suffixes ('_d'))
PYTHON_LIB_DIR_25=$(PYTHON_DIR_25)\libs

# Python folder for 2.6
PYTHON_DIR_26=c:\Python26
# Python executable
PYTHON_EXE_26=$(PYTHON_DIR_26)\python.exe 
 #PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE_26=$(PYTHON_DIR_26)\include
#A 'PC' directory is also included for people building from a source tree.
PYTHON_INCLUDE_2_26=$(PYTHON_DIR_26)\PC

# PYTHON_LIB_DIR : Set this to the directory containing python*.lib
# It should only be necessary to change this for source builds of Python,
# where the files are in 'PCBuild' rather than 'libs' (this magically works
# as Python uses a #pragma to reference the library base name - which
# includes any version numbers and debug suffixes ('_d'))
PYTHON_LIB_DIR_26=$(PYTHON_DIR_26)\libs

# Python folder for 3.0
PYTHON_DIR_30=c:\Program Files\Python30
# Python executable
!if "$(DEBUG)"=="1"
PYTHON_EXE_30=$(PYTHON_DIR_30)\python_d.exe 
!else
PYTHON_EXE_30=$(PYTHON_DIR_30)\python.exe 
!endif
#PYTHON_INCLUDE : Set this to the directory that contains python.h
PYTHON_INCLUDE_30=$(PYTHON_DIR_30)\include
#A 'PC' directory is also included for people building from a source tree.
PYTHON_INCLUDE_2_30=$(PYTHON_DIR_30)\PC

# PYTHON_LIB_DIR : Set this to the directory containing python*.lib
# It should only be necessary to change this for source builds of Python,
# where the files are in 'PCBuild' rather than 'libs' (this magically works
# as Python uses a #pragma to reference the library base name - which
# includes any version numbers and debug suffixes ('_d'))
PYTHON_LIB_DIR_30=$(PYTHON_DIR_30)\libs

# -------------end Python settings-------------


# -------------PHP settings-------------
# PHP source folder
PHP_SRC_DIR=C:\work\php-5.2.1

PHP_INCLUDE_CPPFLAGS= \
-I "$(PHP_SRC_DIR)" -I "$(PHP_SRC_DIR)\tsrm" -I "$(PHP_SRC_DIR)\Zend" -I "$(PHP_SRC_DIR)\main" -I "$(PHP_SRC_DIR)\regex"  \
-D ZTS=1 -D ZEND_WIN32=1 -D PHP_WIN32=1 -D ZEND_WIN32_FORCE_INLINE -D HAVE_WIN32STD=1 \

# version 4 or 5: Define exactly the one you want and leave the other one 
# commented out. Note you will have to modify the paths below as well.
#PHP_MAJOR_VERSION = 4
PHP_MAJOR_VERSION = 5

# PHP_EXE_DIR: Set this to the folder where the PHP executable is
# PHP_LIB : Set this to the path to the PHP library 
!if "$(DEBUG)"=="1"
PHP_EXE_DIR=C:\php-5.2.1\Debug_TS
PHP_LIB=$(PHP_EXE_DIR)\php5ts_debug.lib
PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=1"
!else
PHP_EXE_DIR=C:\work\php-5.2.1-win32
PHP_LIB=$(PHP_EXE_DIR)\dev\php5ts.lib
PHP_DEBUG_OR_RELEASE= /D "ZEND_DEBUG=0"
!endif

#    PHP executable
PHP_EXE=$(PHP_EXE_DIR)\PHP.exe 
# ------------- end PHP settings-------------

# -------------Ruby settings-------------
# Tested with ruby 1.8.6 (the one that is installed using the one click installer 'ruby186-26.exe').
# You have to change the "!=" in first line of RUBY_DIR\lib\ruby\1.8\i386-mswin32\config.h
# to "<=" if using a Visual C++ older than 5.0

# Ruby folder
RUBY_DIR=c:\Ruby
# Ruby executable
RUBY_EXE=$(RUBY_DIR)\bin\ruby.exe 
# RUBY_INCLUDE : Set this to the directory that contains ruby.h
RUBY_INCLUDE=$(RUBY_DIR)\lib\ruby\1.8\i386-mswin32
# RUBY_SO_DIR : Where to install the dll file
RUBY_SO_DIR=$(RUBY_INCLUDE)
# RUBY_RB_DIR : Where to install the .rb file
RUBY_RB_DIR=$(RUBY_INCLUDE)\..
# RUBY_LIB_DIR : Set this to the directory containing msvcrt-ruby18*.lib
RUBY_LIB_DIR=$(RUBY_DIR)\lib
# -------------end Ruby settings-------------

# ------------- Java settings ------------

JAVA_DIR=C:\Program Files\Java\jdk1.6.0_05\bin
JAVA_INCLUDE_DIR=C:\Program Files\Java\jdk1.6.0_05\include
JAVA="$(JAVA_DIR)\java.exe"
JAVAC="$(JAVA_DIR)\javac.exe"
JAR="$(JAVA_DIR)\jar.exe"
JAVA_PATHSEP=/

# ------------- end Java settings-------------

# ------------- C# settings ------------

CSC="$(LIBPATH)\csc.exe"
SN="C:\Program Files\Microsoft Visual Studio .NET 2003\SDK\v1.1\Bin\sn.exe"

# ------------- end C# settings ------------

# ------------SWIG settings-------------
# Swig executable
SWIG=\work\xapian\xapian-svn\swig\swig.exe
SWIG_FLAGS= -Werror 
# ------------end SWIG settings-------------

# ------------ Misc external libraries we depend on -------------
ZLIB_DIR=C:\gnu\zlib123-dll
# If you installed a binary version, the following 3 lines are probably
# correct.  If you build from sources, adjust accordingly.
ZLIB_INCLUDE_DIR=$(ZLIB_DIR)\include
ZLIB_LIB_DIR=$(ZLIB_DIR)\lib
ZLIB_BIN_DIR=$(ZLIB_DIR)

#--------------------------------------
# Visual C++ Compiler and linker programs, and flags for these
#--------------------------------------
LIB32=link.exe -lib
LIB32_FLAGS=-nologo  
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib rpcrt4.lib\
 wsock32.lib Ws2_32.lib  odbccp32.lib -subsystem:console -debug -nologo \
 "$(ZLIB_LIB_DIR)\zdll.lib"
 
CPP=cl.exe
RSC=rc.exe
MANIFEST=mt.exe /manifest

# make sure inference rules work with all source files
.SUFFIXES : .cc .java

# xapdep is a tool to turn compiler output (using the -showIncludes option) into dependency lists
DEPEND=xapdep.exe

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
CPPFLAGS_COMMON=-nologo -c -Zi -I.. -I..\include -I..\common -I..\win32 -W3 -EHsc \
-DWIN32 -D__WIN32__ -D_WIN32 -D_WINDOWS \
-D "HAVE_VSNPRINTF" -D "HAVE_STRDUP" -D "_USE_32BIT_TIME_T" \
-D_CRT_SECURE_NO_DEPRECATE \
-I"$(ZLIB_INCLUDE_DIR)"

# The various parts of Xapian (but *not* the test suite or treecheck libs)
XAPIAN_LIBS = \
 "$(OUTLIBDIR)\libcommon.lib"  \
 "$(OUTLIBDIR)\libbackend.lib"  \
 "$(OUTLIBDIR)\libexpand.lib"  \
 "$(OUTLIBDIR)\libbrass.lib" \
 "$(OUTLIBDIR)\libchert.lib" \
 "$(OUTLIBDIR)\libflint.lib" \
 "$(OUTLIBDIR)\libinmemory.lib" \
 "$(OUTLIBDIR)\libmulti.lib" \
 "$(OUTLIBDIR)\libmatcher.lib"  \
 "$(OUTLIBDIR)\libnet.lib" \
 "$(OUTLIBDIR)\liblanguages.lib"  \
 "$(OUTLIBDIR)\libapi.lib"  \
 "$(OUTLIBDIR)\libremote.lib"  \
 "$(OUTLIBDIR)\libunicode.lib"  \
 "$(OUTLIBDIR)\libweight.lib"  \
 "$(OUTLIBDIR)\libqueryparser.lib"  

!IF "$(DEBUG)" == "1"
# Debug build
CPPFLAGS_EXTRA=$(CPPFLAGS_COMMON) -Od -MDd -D DEBUG -D _DEBUG -D XAPIAN_DEBUG
XAPIAN_DEBUG_OR_RELEASE=Debug
!ELSE
# Release build
CPPFLAGS_EXTRA=$(CPPFLAGS_COMMON) -O2 -MD -D NDEBUG
XAPIAN_DEBUG_OR_RELEASE=Release
!ENDIF

#----------------end Visual C++----------------------

