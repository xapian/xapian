# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen and Charlie Hull
# Modified by Mark Hammond.
# May 2007

# Will build a Win32 static library (non-debug) libexpand.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libexpand.lib" 

LIBEXPAND_OBJS= \
                 $(INTDIR)\ortermlist.obj \
                 $(INTDIR)\expandweight.obj \
                 $(INTDIR)\expand.obj \
                 $(NULL)

CLEAN :
	-@erase "$(OUTDIR)\libexpand.lib"
	-@erase "*.pch"
	-@erase $(LIBEXPAND_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBEXPAND.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBEXPAND_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libexpand.lib" $(DEF_FLAGS) $(LIBEXPAND_OBJS)
<<

"$(INTDIR)\ortermlist.obj" : ".\ortermlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\expandweight.obj" : ".\expandweight.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\expand.obj" : ".\expand.cc"
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

