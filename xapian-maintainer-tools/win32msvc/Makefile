# Xapian makefile for win32 directory

# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd. www.lemurconsulting.com
# Copyright (C) 2007, Olly Betts

!MESSAGE Building Xapian on Win32
!MESSAGE

!INCLUDE .\config.mak

ALL: TESTFORTOOLS MAKEHEADERS COPYMAKFILES MAKEALL

CHECK: ALL DOTEST

MAKEALL:
# Note we must run each make process with the HEADERS target to calculate any missing dependencies and add them to the end of the makefile
   cd ..\common
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" 
   cd ..\api
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\backends
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\docsim
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\matcher
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\languages\compiler
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\queryparser
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\net
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\expand
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\unicode
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\weight
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\geospatial
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\tests\harness
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\..\bin
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\examples
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\tests
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" HEADERS
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"

DOTEST:
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" DOTEST
   cd ..\win32

CLEAN: COPYMAKFILES
   cd xapdep
   nmake -f xapdep.mak CLEAN
   cd ..
   cd ..\net
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\common
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\api
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\backends
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\docsim
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\matcher
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\languages\compiler
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\queryparser
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\expand
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\unicode
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\weight
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\geospatial
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\tests\harness
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\..\bin
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\examples
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\tests
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\win32
   -@erase ..\config.h 
   -@erase ..\include\xapian\version.h
   -@erase ..\include\xapian\error.h
   -@erase ..\include\xapian\errordispatch.h
   if exist "$(XAPIAN_DEBUG_OR_RELEASE)" rmdir $(XAPIAN_DEBUG_OR_RELEASE)\ /s /q
   -@erase xapdep.exe
   echo All Win32 parts have been cleaned!

DISTCLEAN: CLEAN

# Copy mak files to the right folders
COPYMAKFILES: 
   copy config.h.win32 ..\config.h
   copy  win32_api.mak ..\api\Makefile
   copy  win32_backends.mak ..\backends\Makefile
   copy  win32_backends_flint.mak ..\backends\flint\Makefile
   copy  win32_backends_brass.mak ..\backends\brass\Makefile
   copy  win32_backends_chert.mak ..\backends\chert\Makefile
   copy  win32_backends_inmemory.mak ..\backends\inmemory\Makefile
   copy  win32_backends_multi.mak ..\backends\multi\Makefile
   copy  win32_backends_remote.mak ..\backends\remote\Makefile
   copy  win32_bin.mak ..\bin\Makefile
   copy  win32_common.mak ..\common\Makefile
   copy  win32_examples.mak ..\examples\Makefile
   copy  win32_expand.mak ..\expand\Makefile
   copy  win32_languages_compiler.mak ..\languages\compiler\Makefile
   copy  win32_languages.mak ..\languages\Makefile
   copy  win32_docsim.mak ..\docsim\Makefile
   copy  win32_matcher.mak ..\matcher\Makefile
   copy  win32_net.mak ..\net\Makefile
   copy  win32_unicode.mak ..\unicode\Makefile
   copy  win32_weight.mak ..\weight\Makefile
   copy  win32_geospatial.mak ..\geospatial\Makefile
   copy  win32_queryparser.mak ..\queryparser\Makefile
   copy  win32_tests.mak ..\tests\Makefile
   copy  win32_testsuite.mak ..\tests\harness\Makefile
   if exist $(XAPIAN_BINDINGS) copy $(XAPIAN_BINDINGS)\xapian-version.h.in $(XAPIAN_BINDINGS)\xapian-version.h
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_python.mak $(XAPIAN_BINDINGS)\python\Makefile
   if exist $(XAPIAN_BINDINGS) copy doccomments.i $(XAPIAN_BINDINGS)\python
   if exist $(XAPIAN_BINDINGS) copy version.rc.python $(XAPIAN_BINDINGS)\python\version.rc
   if exist $(XAPIAN_BINDINGS) copy setup.py $(XAPIAN_BINDINGS)\python
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_php.mak $(XAPIAN_BINDINGS)\php\Makefile
   if exist $(XAPIAN_BINDINGS) copy version.rc.php $(XAPIAN_BINDINGS)\php\version.rc
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_java-swig.mak $(XAPIAN_BINDINGS)\java-swig\Makefile
   if exist $(XAPIAN_BINDINGS) copy version.rc.java-swig $(XAPIAN_BINDINGS)\java-swig\version.rc
   if exist $(XAPIAN_BINDINGS) copy WriteJavaVersion.java $(XAPIAN_BINDINGS)\java-swig\WriteJavaVersion.java
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_ruby.mak $(XAPIAN_BINDINGS)\ruby\Makefile
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_csharp.mak $(XAPIAN_BINDINGS)\csharp\Makefile
   if exist $(XAPIAN_BINDINGS) copy version.rc.csharp $(XAPIAN_BINDINGS)\csharp\version.rc
   if exist $(XAPIAN_APPLICATIONS) copy win32_applications_omega.mak $(XAPIAN_APPLICATIONS)\Makefile
   if exist $(XAPIAN_APPLICATIONS) copy config.mak $(XAPIAN_APPLICATIONS)
   if exist $(XAPIAN_APPLICATIONS) copy config.h.omega.win32 $(XAPIAN_APPLICATIONS)\config.h
   if exist $(XAPIAN_APPLICATIONS) copy config.h.omega.win32 $(XAPIAN_APPLICATIONS)\common\config.h

# Test for tools that we MUST have   
TESTFORTOOLS: xapdep.exe
# Perl is needed for header generation
   if not exist $(PERL_EXE) exit(1) 
   
MAKEALLBINDINGS: ALL COPYMAKFILES
# Need bindings to exist
   if not exist  $(XAPIAN_BINDINGS) exit(1)

   cd $(XAPIAN_BINDINGS)\python
   nmake PYTHON_VER=24 CLEAN
   nmake PYTHON_VER=24 DIST

   nmake PYTHON_VER=25 CLEAN
   nmake PYTHON_VER=25 DIST
   
   nmake PYTHON_VER=26 CLEAN
   nmake PYTHON_VER=26 DIST
   
   cd $(XAPIAN_BINDINGS)\php
   nmake CLEAN
   nmake DIST
   
   cd $(XAPIAN_BINDINGS)\java-swig
   nmake CLEAN
   nmake DIST

   cd $(XAPIAN_BINDINGS)\ruby
   nmake CLEAN
   nmake DIST
   
   cd $(XAPIAN_BINDINGS)\csharp
   nmake CLEAN
   nmake DIST
   
# Header files generated automatically from various .in files
MAKEHEADERS: ..\include\xapian\error.h ..\include\xapian\version.h

..\include\xapian\error.h: ..\generate-exceptions
    cd..
    $(PERL_EXE) generate-exceptions
    cd win32

..\include\xapian\version.h: version.h.in ..\configure.ac
    $(PERL_EXE) genversion.pl ..\configure.ac version.h.in ..\include\xapian\version.h
    
xapdep.exe: 
    cd xapdep
    nmake -f xapdep.mak
    copy xapdep.exe ..
    cd ..

