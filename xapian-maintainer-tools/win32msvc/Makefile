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
   cd ..\common
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\api
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\backends
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\matcher
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\languages\compiler
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\queryparser
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\net
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\expand
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\unicode
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\tests\harness
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\..\bin
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\examples
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
   cd ..\tests
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"

DOTEST:
   nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)" DOTEST
   cd ..\win32

CLEAN: COPYMAKFILES
   cd ..\net
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\common
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\api
   nmake /$(MAKEFLAGS) DEBUG="$(DEBUG)" CLEAN
   cd ..\backends
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
   -@erase ..\generate-exceptions
   -@erase ..\include\xapian\error.h
   -@erase ..\include\xapian\errordispatch.h
   -@erase $(XAPIAN_DEBUG_OR_RELEASE)\*.idb
   rmdir $(XAPIAN_DEBUG_OR_RELEASE)\ /s /q
   echo All Win32 parts have been cleaned!

DISTCLEAN: CLEAN

# Copy mak files to the right folders
COPYMAKFILES: 
   copy config.h.win32 ..\config.h
   copy  win32_api.mak ..\api\Makefile
   copy  win32_backends.mak ..\backends\Makefile
   copy  win32_backends_flint.mak ..\backends\flint\Makefile
   copy  win32_backends_inmemory.mak ..\backends\inmemory\Makefile
   copy  win32_backends_multi.mak ..\backends\multi\Makefile
   copy  win32_backends_remote.mak ..\backends\remote\Makefile
   copy  win32_backends_quartz.mak ..\backends\quartz\Makefile
   copy  win32_bin.mak ..\bin\Makefile
   copy  win32_common.mak ..\common\Makefile
   copy  win32_examples.mak ..\examples\Makefile
   copy  win32_expand.mak ..\expand\Makefile
   copy  win32_languages_compiler.mak ..\languages\compiler\Makefile
   copy  win32_languages.mak ..\languages\Makefile
   copy  win32_matcher.mak ..\matcher\Makefile
   copy  win32_net.mak ..\net\Makefile
   copy  win32_unicode.mak ..\unicode\Makefile
   copy  win32_queryparser.mak ..\queryparser\Makefile
   copy  win32_tests.mak ..\tests\Makefile
   copy  win32_testsuite.mak ..\tests\harness\Makefile
   if exist $(XAPIAN_BINDINGS) copy $(XAPIAN_BINDINGS)\xapian-version.h.in $(XAPIAN_BINDINGS)\xapian-version.h
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_python.mak $(XAPIAN_BINDINGS)\python\Makefile
   if not exist $(XAPIAN_BINDINGS)\python\doccomments.i copy doccomments.i $(XAPIAN_BINDINGS)\python
   if exist $(XAPIAN_BINDINGS) copy version.rc.python $(XAPIAN_BINDINGS)\python\version.rc
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_php.mak $(XAPIAN_BINDINGS)\php\Makefile
   if exist $(XAPIAN_BINDINGS) copy version.rc.php $(XAPIAN_BINDINGS)\php\version.rc
   if exist $(XAPIAN_APPLICATIONS) copy win32_applications_omega.mak $(XAPIAN_APPLICATIONS)\omega\Makefile
   if exist $(XAPIAN_APPLICATIONS) copy config.mak $(XAPIAN_APPLICATIONS)\omega
   if exist $(XAPIAN_APPLICATIONS) copy config.h.omega.win32 $(XAPIAN_APPLICATIONS)\omega\config.h

# Test for tools that we MUST have   
TESTFORTOOLS:
# Perl is needed for header generation
   if not exist $(PERL_EXE) exit(1) 
   
# Header files generated automatically from various .in files
MAKEHEADERS: ..\include\xapian\error.h ..\include\xapian\version.h

..\generate-exceptions: ..\generate-exceptions.in Makefile
    $(PERL_EXE) -pe "BEGIN{$$perl=shift @ARGV} s,\@PERL\@,$$perl," "$(PERL_EXE)" ..\generate-exceptions.in > ..\generate-exceptions

..\include\xapian\error.h: ..\generate-exceptions
    cd..
    $(PERL_EXE) generate-exceptions
    cd win32

..\include\xapian\version.h: ..\msvc\version.h.in ..\configure.ac
    $(PERL_EXE) ..\msvc\genversion.pl ..\configure.ac ..\msvc\version.h.in ..\include\xapian\version.h

