# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libmatcher.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libmatcher.lib" 

OBJS= \
    $(INTDIR)\andmaybepostlist.obj\
    $(INTDIR)\andnotpostlist.obj\
    $(INTDIR)\branchpostlist.obj\
    $(INTDIR)\collapser.obj\
    $(INTDIR)\exactphrasepostlist.obj\
    $(INTDIR)\externalpostlist.obj\
    $(INTDIR)\localmatch.obj\
    $(INTDIR)\mergepostlist.obj\
    $(INTDIR)\msetcmp.obj\
    $(INTDIR)\msetpostlist.obj\
    $(INTDIR)\multiandpostlist.obj\
    $(INTDIR)\multimatch.obj\
    $(INTDIR)\multixorpostlist.obj\
    $(INTDIR)\orpostlist.obj\
    $(INTDIR)\phrasepostlist.obj\
    $(INTDIR)\queryoptimiser.obj\
    $(INTDIR)\rset.obj\
    $(INTDIR)\selectpostlist.obj\
    $(INTDIR)\synonympostlist.obj\
    $(INTDIR)\valuerangepostlist.obj\
    $(INTDIR)\valuegepostlist.obj\
    $(INTDIR)\valuestreamdocument.obj\
    $(INTDIR)\remotesubmatch.obj


SRCS= \
    $(INTDIR)\andmaybepostlist.cc\
    $(INTDIR)\andnotpostlist.cc\
    $(INTDIR)\branchpostlist.cc\
    $(INTDIR)\collapser.cc\
    $(INTDIR)\exactphrasepostlist.cc\
    $(INTDIR)\externalpostlist.cc\
    $(INTDIR)\localmatch.cc\
    $(INTDIR)\mergepostlist.cc\
    $(INTDIR)\msetcmp.cc\
    $(INTDIR)\msetpostlist.cc\
    $(INTDIR)\multiandpostlist.cc\
    $(INTDIR)\multimatch.cc\
    $(INTDIR)\multixorpostlist.cc\
    $(INTDIR)\orpostlist.cc\
    $(INTDIR)\phrasepostlist.cc\
    $(INTDIR)\queryoptimiser.cc\
    $(INTDIR)\rset.cc\
    $(INTDIR)\selectpostlist.cc\
    $(INTDIR)\synonympostlist.cc\
    $(INTDIR)\valuerangepostlist.cc\
    $(INTDIR)\valuegepostlist.cc\
    $(INTDIR)\valuestreamdocument.cc\
    $(INTDIR)\remotesubmatch.cc

CLEAN :
    -@erase "$(OUTDIR)\libmatcher.lib"
    -@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
    -@erase "$(INTDIR)\*.pdb"
    -@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBMATCHER.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libmatcher.lib" $(DEF_FLAGS) $(OBJS)
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