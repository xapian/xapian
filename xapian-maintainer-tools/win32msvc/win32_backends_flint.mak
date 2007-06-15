# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libflint.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libflint.lib" 

LIBFLINT_OBJS= \
               $(INTDIR)\flint_database.obj \
               $(INTDIR)\flint_termlist.obj \
               $(INTDIR)\flint_postlist.obj \
               $(INTDIR)\flint_positionlist.obj \
               $(INTDIR)\flint_record.obj \
               $(INTDIR)\flint_values.obj \
               $(INTDIR)\flint_document.obj \
               $(INTDIR)\flint_alltermslist.obj \
	       $(INTDIR)\flint_alldocspostlist.obj \
               $(INTDIR)\flint_table.obj \
               $(INTDIR)\flint_cursor.obj \
               $(INTDIR)\flint_btreebase.obj \
               $(INTDIR)\flint_version.obj \
	       $(INTDIR)\flint_io.obj \
               $(INTDIR)\flint_modifiedpostlist.obj \
               $(INTDIR)\flint_lock.obj
              
              
LOCAL_HEADERS =\
	$(INTDIR)\flint_alldocspostlist.h\
	$(INTDIR)\flint_alltermslist.h\
	$(INTDIR)\flint_btreebase.h\
	$(INTDIR)\flint_btreeutil.h\
	$(INTDIR)\flint_check.h\
	$(INTDIR)\flint_cursor.h\
	$(INTDIR)\flint_database.h\
	$(INTDIR)\flint_document.h\
	$(INTDIR)\flint_io.h\
	$(INTDIR)\flint_lock.h\
	$(INTDIR)\flint_modifiedpostlist.h\
	$(INTDIR)\flint_positionlist.h\
	$(INTDIR)\flint_postlist.h\
	$(INTDIR)\flint_record.h\
	$(INTDIR)\flint_table.h\
	$(INTDIR)\flint_termlist.h\
	$(INTDIR)\flint_types.h\
	$(INTDIR)\flint_utils.h\
	$(INTDIR)\flint_values.h\
	$(INTDIR)\flint_version.h         

CLEAN :
	-@erase "$(OUTDIR)\libflint.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBFLINT_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBFLINT.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBFLINT_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libflint.lib" $(DEF_FLAGS) $(LIBFLINT_OBJS)
<<

# if any headers change, rebuild all .objs
$(LIBFLINT_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

