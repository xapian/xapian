# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libmulti.lib


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\..\win32\config.mak


CPP=cl.exe
RSC=rc.exe


OUTDIR=..\..\win32\Release\libs
INTDIR=.\

ALL : "$(OUTDIR)\libmulti.lib" 

LIBMULTI_OBJS= \
                $(INTDIR)\multi_postlist.obj \
                $(INTDIR)\multi_termlist.obj \
                $(INTDIR)\multi_alltermslist.obj

CLEAN :
	-@erase "$(OUTDIR)\libmulti.lib"
	-@erase "*.pch"
        -@erase $(LIBMULTI_OBJS)
	-@erase $(LIBMULTI_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) /W3 /GX /O2 \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /Fo"$(INTDIR)\\" \
 /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"
CPP_OBJS=..\..\win32\Release
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)


"$(OUTDIR)\LIBMULTI.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBMULTI_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libmulti.lib" $(DEF_FLAGS) $(LIBMULTI_OBJS)
<<


"$(INTDIR)\multi_postlist.obj" : "multi_postlist.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\multi_termlist.obj" : "multi_termlist.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\multi_alltermslist.obj" : "multi_alltermslist.cc"
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

