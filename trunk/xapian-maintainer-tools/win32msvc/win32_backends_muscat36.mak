# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libmuscat36.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libmuscat36.lib" 

LIBMUSCAT36_OBJS= \
                $(INTDIR)\3point6.obj \
                $(INTDIR)\daread.obj \
                $(INTDIR)\da_alltermslist.obj \
                $(INTDIR)\da_database.obj \
                $(INTDIR)\da_document.obj \
                $(INTDIR)\dbread.obj \
                $(INTDIR)\db_database.obj \
                $(INTDIR)\db_document.obj \
                $(INTDIR)\io_system.obj

CLEAN :
	-@erase "$(OUTDIR)\libmuscat36.lib"
	-@erase "*.pch"
        -@erase $(LIBMULTI_OBJS)
	-@erase $(LIBMUSCAT36_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\libmuscat36.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBMUSCAT36_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libmuscat36.lib" $(DEF_FLAGS) $(LIBMUSCAT36_OBJS)
<<


"$(INTDIR)\3point6.obj" : "3point6.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\daread.obj" : "daread.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\da_alltermslist.obj" : "da_alltermslist.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\da_database.obj" : "da_database.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\da_document.obj" : "da_document.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\dbread.obj" : "dbread.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\db_database.obj" : "db_database.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\db_document.obj" : "db_document.cc"
    $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\io_system.obj" : "io_system.cc"
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

