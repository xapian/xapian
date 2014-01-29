# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libquartz.lib


!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libquartz.lib" "$(OUTDIR)\libquartzbtreecheck.lib" 


LIBQUARTZBTREECHECK_OBJS= \
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
                $(INTDIR)\quartz_alldocspostlist.obj \
                $(INTDIR)\quartz_metafile.obj \
                $(INTDIR)\btree.obj \
                $(INTDIR)\bcursor.obj \
                $(INTDIR)\btree_base.obj

SRCS= \
                $(INTDIR)\quartz_database.cc \
                $(INTDIR)\quartz_termlist.cc \
                $(INTDIR)\quartz_postlist.cc \
                $(INTDIR)\quartz_positionlist.cc \
                $(INTDIR)\quartz_record.cc \
                $(INTDIR)\quartz_values.cc \
                $(INTDIR)\quartz_log.cc \
                $(INTDIR)\quartz_document.cc \
                $(INTDIR)\quartz_alltermslist.cc \
                $(INTDIR)\quartz_alldocspostlist.cc \
                $(INTDIR)\quartz_metafile.cc \
                $(INTDIR)\btree.cc \
                $(INTDIR)\bcursor.cc \
                $(INTDIR)\btree_base.cc \
                $(INTDIR)\btreecheck.cc 

CLEAN :
    -@erase "$(OUTDIR)\libquartz.lib"
    -@erase "$(OUTDIR)\libquartzbtreecheck.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase "$(INTDIR)\*.pdb"
    -@erase $(LIBQUARTZBTREECHECK_OBJS)
    -@erase $(LIBQUARTZ_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I "..\.." -I "..\..\include" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBQUARTZ.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBQUARTZ_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libquartz.lib" $(DEF_FLAGS) $(LIBQUARTZ_OBJS)
<<


"$(OUTDIR)\LIBQUARTZBTREECHECK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBQUARTZBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libquartzbtreecheck.lib" $(DEF_FLAGS) $(LIBQUARTZBTREECHECK_OBJS)
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