# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libbtreecheck.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

BUILD_LIBRARIES = "$(OUTDIR)\libtest.lib"

ALL : $(BUILD_LIBRARIES) 

OBJS= \
        $(INTDIR)\testrunner.obj \
        $(INTDIR)\testsuite.obj \
        $(INTDIR)\testutils.obj \
        $(INTDIR)\backendmanager.obj \
        $(INTDIR)\backendmanager_flint.obj \
        $(INTDIR)\backendmanager_brass.obj \
        $(INTDIR)\backendmanager_chert.obj \
        $(INTDIR)\backendmanager_inmemory.obj \
        $(INTDIR)\backendmanager_multi.obj \
        $(INTDIR)\backendmanager_remote.obj \
        $(INTDIR)\backendmanager_remoteprog.obj \
        $(INTDIR)\backendmanager_remotetcp.obj \
        $(INTDIR)\cputimer.obj \
        $(INTDIR)\index_utils.obj \
        $(INTDIR)\unixcmds.obj

SRCS= \
        $(INTDIR)\testrunner.cc \
        $(INTDIR)\testsuite.cc \
        $(INTDIR)\testutils.cc \
        $(INTDIR)\backendmanager.cc \
        $(INTDIR)\backendmanager_flint.cc \
        $(INTDIR)\backendmanager_brass.cc \
        $(INTDIR)\backendmanager_chert.cc \
        $(INTDIR)\backendmanager_inmemory.cc \
        $(INTDIR)\backendmanager_multi.cc \
        $(INTDIR)\backendmanager_remote.cc \
        $(INTDIR)\backendmanager_remoteprog.cc \
        $(INTDIR)\backendmanager_remotetcp.cc \
        $(INTDIR)\cputimer.cc \
        $(INTDIR)\index_utils.cc \
        $(INTDIR)\unixcmds.cc

CLEAN :
        -@erase $(BUILD_LIBRARIES)
        -@erase "*.pch"
        -@erase "$(INTDIR)\*.pdb"
        -@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\.." -I"..\..\include" -I"..\..\api" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBTEST.lib" : HEADERS  "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libtest.lib" $(DEF_FLAGS) $(OBJS)
<<

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

# Calculate any header dependencies and automatically insert them into this file
HEADERS :
            if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) $(DEPEND_FLAGS) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)" 
# DO NOT DELETE THIS LINE -- make depend depends on it.
