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

LIBMATCHER_OBJS= \
                 $(INTDIR)\orpostlist.obj \
                 $(INTDIR)\andpostlist.obj \
                 $(INTDIR)\andnotpostlist.obj \
                 $(INTDIR)\andmaybepostlist.obj \
                 $(INTDIR)\xorpostlist.obj \
                 $(INTDIR)\phrasepostlist.obj \
                 $(INTDIR)\selectpostlist.obj \
                 $(INTDIR)\filterpostlist.obj \
                 $(INTDIR)\rset.obj \
                 $(INTDIR)\bm25weight.obj \
                 $(INTDIR)\tradweight.obj \
                 $(INTDIR)\localmatch.obj \
                 $(INTDIR)\multimatch.obj \
                 $(INTDIR)\stats.obj \
                 $(INTDIR)\mergepostlist.obj \
                 $(INTDIR)\msetpostlist.obj \
		 $(INTDIR)\msetcmp.obj \
                 $(INTDIR)\emptysubmatch.obj \
                 $(INTDIR)\exactphrasepostlist.obj \
                 $(INTDIR)\valuerangepostlist.obj \
                 $(INTDIR)\weight.obj \
                 $(INTDIR)\remotesubmatch.obj \
                 $(NULL)

LOCAL_HEADERS =\
	$(INTDIR)\andmaybepostlist.h\
	$(INTDIR)\andnotpostlist.h\
	$(INTDIR)\andpostlist.h\
	$(INTDIR)\branchpostlist.h\
	$(INTDIR)\emptysubmatch.h\
	$(INTDIR)\exactphrasepostlist.h\
	$(INTDIR)\extraweightpostlist.h\
	$(INTDIR)\filterpostlist.h\
	$(INTDIR)\localmatch.h\
	$(INTDIR)\mergepostlist.h\
	$(INTDIR)\msetcmp.h\
	$(INTDIR)\msetpostlist.h\
	$(INTDIR)\orpostlist.h\
	$(INTDIR)\phrasepostlist.h\
	$(INTDIR)\remotesubmatch.h\
	$(INTDIR)\selectpostlist.h\
	$(INTDIR)\valuerangepostlist.h\
	$(INTDIR)\xorpostlist.h

CLEAN :
	-@erase "$(OUTDIR)\libmatcher.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBMATCHER_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBMATCHER.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBMATCHER_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libmatcher.lib" $(DEF_FLAGS) $(LIBMATCHER_OBJS)
<<

# if any headers change, rebuild all .objs
$(LIBMATCHER_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

