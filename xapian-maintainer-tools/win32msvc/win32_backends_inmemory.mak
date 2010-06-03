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

OBJS= \
                 $(INTDIR)\inmemory_database.obj \
                 $(INTDIR)\inmemory_document.obj \
                 $(INTDIR)\inmemory_positionlist.obj \
                 $(INTDIR)\inmemory_alltermslist.obj 
SRCS= \
                 $(INTDIR)\inmemory_database.cc \
                 $(INTDIR)\inmemory_document.cc \
                 $(INTDIR)\inmemory_positionlist.cc \
                 $(INTDIR)\inmemory_alltermslist.cc 
		 
CLEAN :
	-@erase "$(OUTDIR)\libinmemory.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
	-@erase "$(INTDIR)\*.pdb"
	-@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)-$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 -I "..\.." -I "..\..\include" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME) 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBINMEMORY.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libinmemory.lib" $(DEF_FLAGS) $(OBJS)
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
    if exist "..\..\win32\$(DEPEND)" ..\..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.