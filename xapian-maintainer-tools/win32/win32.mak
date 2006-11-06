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
   cd ..\languages
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\queryparser
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\testsuite
   nmake /f win32.mak $(MAKEMACRO) CFG="$(CFG)"
   cd ..\bin
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
   cd ..\languages
   nmake /f win32.mak CLEAN
   cd ..\queryparser
   nmake /f win32.mak CLEAN
   cd ..\testsuite
   nmake /f win32.mak CLEAN
   cd ..\bin
   nmake /f win32.mak CLEAN
   cd ..\examples
   nmake /f win32.mak CLEAN
   cd ..\tests
   nmake /f win32.mak CLEAN
   cd ..\win32
   -@erase ..\config.h 
   -@erase ..\include\xapian\version.h
   -@erase ..\include\unistd.h
   -@erase Release\*.idb
#   -@rmdir Release\ /s /q
   echo All Win32 parts have been cleaned!

DISTCLEAN: CLEAN


