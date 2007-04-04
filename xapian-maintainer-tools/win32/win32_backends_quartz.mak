# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libquartz.lib


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\..\win32\config.mak


CPP=cl.exe
RSC=rc.exe


OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libquartz.lib" "$(OUTDIR)\libbtreecheck.lib" 


LIBBTREECHECK_OBJS= \
                $(INTDIR)\btreecheck.obj

LIBQUARTZ_OBJS= \
                $(INTDIR)\quartz_database.obj \
                $(INTDIR)\quartz_termlist.obj \
                $(INTDIR)\quartz_postlist.obj \
                $(INTDIR)\quartz_positionlist.obj \
                $(INTDIR)\quartz_record.obj \
                $(INTDIR)\quartz_values.obj \
                $(INTDIR)\quartz_log.obj \
                $(INTDIR)\quartz_document.obj \
                $(INTDIR)\quartz_alltermslist.obj \
                $(INTDIR)\quartz_metafile.obj \
                $(INTDIR)\btree.obj \
                $(INTDIR)\bcursor.obj \
                $(INTDIR)\btree_base.obj


CLEAN :
	-@erase "$(OUTDIR)\libquartz.lib"
	-@erase "$(OUTDIR)\libbtreecheck.lib"
	-@erase "*.pch"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase $(LIBBTREECHECK_OBJS)
	-@erase $(LIBQUARTZ_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)

"$(OUTDIR)\LIBQUARTZ.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBQUARTZ_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libquartz.lib" $(DEF_FLAGS) $(LIBQUARTZ_OBJS)
<<


"$(OUTDIR)\LIBBTREECHECK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libbtreecheck.lib" $(DEF_FLAGS) $(LIBBTREECHECK_OBJS)
<<


"$(INTDIR)\btreecheck.obj" : ".\btreecheck.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_database.obj" : "quartz_database.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_termlist.obj" : "quartz_termlist.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\quartz_postlist.obj" : "quartz_postlist.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\quartz_positionlist.obj" : "quartz_positionlist.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_record.obj" : "quartz_record.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_values.obj" : "quartz_values.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_log.obj" : "quartz_log.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_document.obj" : "quartz_document.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_alltermslist.obj" : "quartz_alltermslist.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartz_metafile.obj" : "quartz_metafile.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\btree.obj" : "btree.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\bcursor.obj" : "bcursor.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\btree_base.obj" : "btree_base.cc"
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

