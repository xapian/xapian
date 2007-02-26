# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libnet.lib


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

ALL : "$(OUTDIR)\libnet.lib" 

LIBNET_OBJS= \
                 $(INTDIR)\serialise.obj
				 
CLEAN :
	-@erase "$(OUTDIR)\libnet.lib"
	-@erase "*.pch"
        -@erase $(LIBNET_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) /W3 /GX /O2 \
 /I ".." /I "..\include" /I"..\common" /I"..\net" \
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /Fo"$(INTDIR)\\" \
 /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"
CPP_OBJS=..\win32\Release
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)



"$(OUTDIR)\LIBNET.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBNET_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libnet.lib" $(DEF_FLAGS) $(LIBNET_OBJS)
<<


"$(INTDIR)\serialise.obj" : ".\serialise.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<
