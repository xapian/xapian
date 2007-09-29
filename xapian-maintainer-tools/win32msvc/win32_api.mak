# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com

# Will build a Win32 static library (non-debug) libapi.lib



!INCLUDE ..\win32\config.mak


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libapi.lib" 

OBJS= \
             $(INTDIR)\error.obj \
             $(INTDIR)\errorhandler.obj \
             $(INTDIR)\expanddecider.obj \
             $(INTDIR)\omenquire.obj  \
             $(INTDIR)\omquery.obj  \
             $(INTDIR)\omqueryinternal.obj  \
             $(INTDIR)\omdatabase.obj  \
             $(INTDIR)\omdocument.obj  \
             $(INTDIR)\ompostlistiterator.obj  \
             $(INTDIR)\ompositionlistiterator.obj  \
             $(INTDIR)\omtermlistiterator.obj  \
             $(INTDIR)\omvalueiterator.obj \
             $(INTDIR)\termlist.obj \
             $(INTDIR)\valuerangeproc.obj \
	     $(INTDIR)\version.obj \
	     $(INTDIR)\editdistance.obj \
	     $(INTDIR)\valuerangeproccompat.obj \
	     $(INTDIR)\sortable-serialise.obj 
SRCS= \
             $(INTDIR)\error.cc \
             $(INTDIR)\errorhandler.cc \
             $(INTDIR)\expanddecider.cc \
             $(INTDIR)\omenquire.cc  \
             $(INTDIR)\omquery.cc  \
             $(INTDIR)\omqueryinternal.cc  \
             $(INTDIR)\omdatabase.cc  \
             $(INTDIR)\omdocument.cc  \
             $(INTDIR)\ompostlistiterator.cc  \
             $(INTDIR)\ompositionlistiterator.cc  \
             $(INTDIR)\omtermlistiterator.cc  \
             $(INTDIR)\omvalueiterator.cc \
             $(INTDIR)\termlist.cc \
             $(INTDIR)\valuerangeproc.cc \
	     $(INTDIR)\version.cc \
	     $(INTDIR)\editdistance.cc  \
	     $(INTDIR)\valuerangeproccompat.cc \
	     $(INTDIR)\sortable-serialise.cc 

	     
CLEAN :
	-@erase "$(OUTDIR)\libapi.lib"
	-@erase "*.pch"
	-@erase "$(INTDIR)\*.pdb"
        -@erase $(OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBAPI.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libapi.lib" $(DEF_FLAGS) $(OBJS)
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
            ..\win32\$(DEPEND) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)"
# DO NOT DELETE THIS LINE -- make depend depends on it.
