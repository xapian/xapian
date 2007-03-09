# Xapian makefile for win32 directory

# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com

!MESSAGE Building Xapian on Win32
!MESSAGE

!INCLUDE .\config.mak

ALL: TESTFORTOOLS MAKEHEADERS COPYMAKFILES MAKEALL

MAKEALL:
   cd ..\getopt
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\common
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\api
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\backends
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\matcher
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\languages\compiler
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\queryparser
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\tests\harness
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\..\bin
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\examples
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\tests
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
# NOTE THAT THIS MUST BE DONE LAST! running tests depends on all other parts being built   
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)" DOTEST
   cd ..\win32

CLEAN:
# cd ..\net
#   nmake /f win32.mak CLEAN
   cd ..\getopt
   nmake /f win32.mak CLEAN
   cd ..\common
   nmake /f win32.mak CLEAN
   cd ..\api
   nmake /f win32.mak CLEAN
   cd ..\backends
   nmake /f win32.mak CLEAN
   cd ..\matcher
   nmake /f win32.mak CLEAN
   cd ..\languages\compiler
   nmake /f win32.mak CLEAN
   cd ..
   nmake /f win32.mak CLEAN
   cd ..\queryparser
   nmake /f win32.mak CLEAN
   cd ..\tests\harness
   nmake /f win32.mak CLEAN
   cd ..\..\bin
   nmake /f win32.mak CLEAN
   cd ..\examples
   nmake /f win32.mak CLEAN
   cd ..\tests
   nmake /f win32.mak CLEAN
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
   copy version.h.win32 ..\include\xapian\version.h
   copy  win32_api.mak ..\api\win32.mak
   copy  win32_backends.mak ..\backends\win32.mak
   copy  win32_backends_flint.mak ..\backends\flint\win32.mak
   copy  win32_backends_inmemory.mak ..\backends\inmemory\win32.mak
   copy  win32_backends_multi.mak ..\backends\multi\win32.mak
   copy  win32_backends_remote.mak ..\backends\remote\win32.mak
   copy  win32_backends_quartz.mak ..\backends\quartz\win32.mak
   copy  win32_bin.mak ..\bin\win32.mak
   copy  win32_common.mak ..\common\win32.mak
   copy  win32_examples.mak ..\examples\win32.mak
   copy  win32_getopt.mak ..\getopt\win32.mak
   copy  win32_languages_compiler.mak ..\languages\compiler\win32.mak
   copy  win32_languages.mak ..\languages\win32.mak
   copy  win32_matcher.mak ..\matcher\win32.mak
   copy  win32_queryparser.mak ..\queryparser\win32.mak
   copy  win32_tests.mak ..\tests\win32.mak
   copy  win32_testsuite.mak ..\tests\harness\win32.mak
   if exist $(XAPIAN_BINDINGS) copy $(XAPIAN_BINDINGS)\xapian-version.h.in $(XAPIAN_BINDINGS)\xapian-version.h
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_python.mak $(XAPIAN_BINDINGS)\python\win32.mak
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_php.mak $(XAPIAN_BINDINGS)\php\win32.mak
   if exist $(XAPIAN_APPLICATIONS) copy win32_applications_omega.mak $(XAPIAN_APPLICATIONS)\omega\win32.mak
   if exist $(XAPIAN_APPLICATIONS) copy config.mak $(XAPIAN_APPLICATIONS)\omega
   if exist $(XAPIAN_APPLICATIONS) copy config.h.omega.win32 $(XAPIAN_APPLICATIONS)\omega\config.h

# Test for tools that we MUST have   
TESTFORTOOLS:
# Perl is needed for header generation
   if not exist $(PERL_EXE) exit(1) 
   
# Header files generated automatically from various .in files
MAKEHEADERS: ..\include\xapian\error.h 

..\generate-exceptions: ..\generate-exceptions.in
    $(PERL_EXE) -pe 's,$(PERL_DIR),$(PERL_EXE),' ..\generate-exceptions.in > ..\generate-exceptions

..\include\xapian\error.h: ..\generate-exceptions
    cd..
    $(PERL_EXE) generate-exceptions
    cd win32
       
