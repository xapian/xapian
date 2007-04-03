# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006


# Will build a Win32 static library (non-debug) libinmemory.lib


!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libinmemory.lib" 

LIBINMEMORY_OBJS= \
                 $(INTDIR)\inmemory_database.obj \
                 $(INTDIR)\inmemory_document.obj \
                 $(INTDIR)\inmemory_positionlist.obj \
                 $(INTDIR)\inmemory_alltermslist.obj \

CLEAN :
	-@erase "$(OUTDIR)\libinmemory.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBINMEMORY_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME) 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBINMEMORY.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBINMEMORY_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libinmemory.lib" $(DEF_FLAGS) $(LIBINMEMORY_OBJS)
<<



"$(INTDIR)\inmemory_database.obj" : "inmemory_database.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<
   
"$(INTDIR)\inmemory_document.obj" : "inmemory_document.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\inmemory_positionlist.obj" : "inmemory_positionlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\inmemory_alltermslist.obj" : "inmemory_alltermslist.cc"
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

