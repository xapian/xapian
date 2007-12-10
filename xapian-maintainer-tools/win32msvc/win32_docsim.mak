# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Modified by Richard Boulton, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 10th December 2007

# Will build a Win32 static library (non-debug) libdocsim.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libdocsim.lib" 

OBJS= \
    $(INTDIR)\docsim.obj\
    $(INTDIR)\docsim_cosine.obj


SRCS= \
    $(INTDIR)\docsim.cc\
    $(INTDIR)\docsim_cosine.cc


CLEAN :
    -@erase "$(OUTDIR)\libdocsim.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\libdocsim.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libdocsim.lib" $(DEF_FLAGS) $(OBJS)
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
            if exist ..\win32\$(DEPEND) ..\win32\$(DEPEND) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)"
# DO NOT DELETE THIS LINE -- make depend depends on it.
