# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libbtreecheck.lib


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

BUILD_LIBRARIES = "$(OUTDIR)\libbtreecheck.lib" "$(OUTDIR)\libtest.lib"


ALL : $(BUILD_LIBRARIES) 

LIBBTREECHECK_OBJS= \
                $(INTDIR)\btreecheck.obj

LIBTEST_OBJS= \
                $(INTDIR)\testsuite.obj \
                $(INTDIR)\testutils.obj \
                $(INTDIR)\backendmanager.obj \
                $(INTDIR)\index_utils.obj

CLEAN :
	-@erase $(BUILD_LIBRARIES)
	-@erase "*.pch"
        -@erase $(LIBBTREECHECK_OBJS)
        -@erase $(LIBTEST_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) /W3 /GX /O2 \
 /I".." /I"..\include" /I"..\api" /I"..\common" /I"..\languages" \
 /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /Fo"$(INTDIR)\\" \
 /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"
CPP_OBJS=..\win32\Release
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)




"$(OUTDIR)\LIBBTREECHECK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libbtreecheck.lib" $(DEF_FLAGS) $(LIBBTREECHECK_OBJS)
<<


"$(OUTDIR)\LIBTEST.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBTEST_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libtest.lib" $(DEF_FLAGS) $(LIBTEST_OBJS)
<<



"$(INTDIR)\btreecheck.obj" : ".\btreecheck.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\testsuite.obj" : ".\testsuite.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\testutils.obj" : ".\testutils.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\backendmanager.obj" : ".\backendmanager.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\index_utils.obj" : ".\index_utils.cc"
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

