# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libcommon.lib


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\win32\config.mak

CPP=cl.exe
RSC=rc.exe


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libcommon.lib" 

LIBCOMMON_OBJS= \
	$(INTDIR)\utils.obj \
	$(INTDIR)\omdebug.obj \
	$(INTDIR)\omstringstream.obj \
	$(INTDIR)\serialise-double.obj  \
	$(INTDIR)\msvc_posix_wrapper.obj
    
CLEAN :
	-@erase "$(OUTDIR)\libcommon.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBCOMMON_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)
   
# Only build netutils if doing QUARTZ backend.


"$(OUTDIR)\LIBCOMMON.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBCOMMON_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libcommon.lib" $(DEF_FLAGS) $(LIBCOMMON_OBJS)
<<

"$(INTDIR)\utils.obj" : ".\utils.cc"
	$(CPP) @<< 
   $(CPP_PROJ) $**
<<

"$(INTDIR)\omdebug.obj" : ".\omdebug.cc"
	$(CPP) @<< 
   $(CPP_PROJ) $**
<<

"$(INTDIR)\omstringstream.obj" : ".\omstringstream.cc"
	$(CPP) @<< 
   $(CPP_PROJ) $**
<<

"$(INTDIR)\serialise-double.obj" : ".\serialise-double.cc"
	$(CPP) @<< 
   $(CPP_PROJ) $**
<<

"$(INTDIR)\msvc_posix_wrapper.obj" : ".\msvc_posix_wrapper.cc"
	$(CPP) @<< 
   $(CPP_PROJ) $**
<<

{.}.cc{$(INTDIR)}.obj:
	$(CPP) /Tc@<<
	$(CPP_PROJ) /Tc$< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

