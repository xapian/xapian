# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libgetopt.lib


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\win32\config.mak

CPP=cl.exe
RSC=rc.exe


OUTDIR=..\win32\Release\libs
INTDIR=.\

ALL : "$(OUTDIR)\libgetopt.lib" 

CLEAN :
	-@erase "$(OUTDIR)\libgetopt.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 

 
CPP_OBJS=..\win32\Release
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)
LIBGETOPT_OBJS= \
	$(INTDIR)\getopt.obj \


"$(OUTDIR)\LIBGETOPT.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBGETOPT_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libgetopt.lib" $(DEF_FLAGS) $(LIBGETOPT_OBJS)
<<

"$(INTDIR)\getopt.obj" : ".\getopt.cc"
	$(CPP) @<< 
   $(CPP_PROJ) $**
<<

{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

