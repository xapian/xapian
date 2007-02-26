# Xapian makefile for win32 directory

# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!MESSAGE Building Xapian on Win32
!MESSAGE

!INCLUDE .\config.mak

ALL: 
   copy config.h.win32 ..\config.h
   copy version.h.win32 ..\include\xapian\version.h
   copy unistd.h.win32 ..\include\unistd.h
   copy error.h.win32 ..\include\xapian\error.h
   copy errordispatch.h.win32 ..\include\xapian\errordispatch.h
   copy allsnowballheaders.h.win32 ..\languages\allsnowballheaders.h
   copy  win32_api.mak ..\api\win32.mak
   copy  win32_backends.mak ..\backends\win32.mak
   copy  win32_backends_flint.mak ..\backends\flint\win32.mak
   copy  win32_backends_inmemory.mak ..\backends\inmemory\win32.mak
   copy  win32_backends_multi.mak ..\backends\multi\win32.mak
#   rem copy  win32_backends_muscat36.mak ..\backends\muscat36\win32.mak
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
   if exist $(XAPIAN_BINDINGS) copy win32_bindings_python.mak $(XAPIAN_BINDINGS)\python\win32.mak
   if exist $(XAPIAN_APPLICATIONS) copy win32_applications_omega.mak $(XAPIAN_APPLICATIONS)\omega\win32.mak
   if exist $(XAPIAN_APPLICATIONS) copy config.mak $(XAPIAN_APPLICATIONS)\omega
   if exist $(XAPIAN_APPLICATIONS) copy config.h.omega.win32 $(XAPIAN_APPLICATIONS)\omega\config.h

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
   -@erase ..\include\unistd.h
   -@erase ..\include\xapian\error.h
   -@erase ..\include\xapian\errordispatch.h
   -@erase ..\languages\allsnowballheaders.h
   -@erase Release\*.idb
#   -@rmdir Release\ /s /q
   echo All Win32 parts have been cleaned!

DISTCLEAN: CLEAN


