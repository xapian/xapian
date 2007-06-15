# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libquartz.lib


!INCLUDE ..\..\win32\config.mak

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
				$(INTDIR)\quartz_alldocspostlist.obj \
                $(INTDIR)\quartz_metafile.obj \
                $(INTDIR)\btree.obj \
                $(INTDIR)\bcursor.obj \
                $(INTDIR)\btree_base.obj
		
LOCAL_HEADERS =\
	$(INTDIR)\bcursor.h\
	$(INTDIR)\btree_base.h\
	$(INTDIR)\btreecheck.h\
	$(INTDIR)\btree.h\
	$(INTDIR)\btree_util.h\
	$(INTDIR)\quartz_alldocspostlist.h\
	$(INTDIR)\quartz_alltermslist.h\
	$(INTDIR)\quartz_database.h\
	$(INTDIR)\quartz_document.h\
	$(INTDIR)\quartz_log.h\
	$(INTDIR)\quartz_metafile.h\
	$(INTDIR)\quartz_positionlist.h\
	$(INTDIR)\quartz_postlist.h\
	$(INTDIR)\quartz_record.h\
	$(INTDIR)\quartz_termlist.h\
	$(INTDIR)\quartz_types.h\
	$(INTDIR)\quartz_utils.h\
	$(INTDIR)\quartz_values.h		


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

"$(OUTDIR)\LIBQUARTZ.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBQUARTZ_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libquartz.lib" $(DEF_FLAGS) $(LIBQUARTZ_OBJS)
<<


"$(OUTDIR)\LIBBTREECHECK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libbtreecheck.lib" $(DEF_FLAGS) $(LIBBTREECHECK_OBJS)
<<

# if any headers change, rebuild all .objs
$(LIBQUARTZ_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

