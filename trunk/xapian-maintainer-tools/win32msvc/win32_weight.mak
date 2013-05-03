# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 25th March 2009

# Will build a Win32 static library (non-debug) libweight.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

OBJS= \
	$(INTDIR)\bm25weight.obj\
	$(INTDIR)\boolweight.obj\
	$(INTDIR)\tradweight.obj\
	$(INTDIR)\weight.obj\
	$(INTDIR)\weightinternal.obj
    
SRCS= \
	$(INTDIR)\bm25weight.cc\
	$(INTDIR)\boolweight.cc\
	$(INTDIR)\tradweight.cc\
	$(INTDIR)\weight.cc\
	$(INTDIR)\weightinternal.cc

CPP_PROJ=$(CPPFLAGS_EXTRA) -I..\win32\ -Fo"$(INTDIR)\\" -Tp$(INPUTNAME) 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL : "$(OUTDIR)\libweight.lib" 

CLEAN :
    -@erase "$(OUTDIR)\libweight.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase $(OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(OUTDIR)\LIBWEIGHT.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS) 
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libweight.lib" $(DEF_FLAGS) $(OBJS)
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
    -@erase deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(SRCS) >>deps.d
    if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.