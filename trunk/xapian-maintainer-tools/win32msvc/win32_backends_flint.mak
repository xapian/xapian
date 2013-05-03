# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libflint.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libflint.lib"  "$(OUTDIR)\libflintbtreecheck.lib" 

LIBFLINTBTREECHECK_OBJS= \
                $(INTDIR)\flint_check.obj

OBJS= \
                $(INTDIR)\flint_alldocspostlist.obj\
                $(INTDIR)\flint_alltermslist.obj\
                $(INTDIR)\flint_btreebase.obj\
                $(INTDIR)\flint_compact.obj\
                $(INTDIR)\flint_cursor.obj\
                $(INTDIR)\flint_database.obj\
                $(INTDIR)\flint_databasereplicator.obj\
                $(INTDIR)\flint_document.obj\
                $(INTDIR)\flint_metadata.obj\
                $(INTDIR)\flint_modifiedpostlist.obj\
                $(INTDIR)\flint_positionlist.obj\
                $(INTDIR)\flint_postlist.obj\
                $(INTDIR)\flint_record.obj\
                $(INTDIR)\flint_spelling.obj\
                $(INTDIR)\flint_spellingwordslist.obj\
                $(INTDIR)\flint_synonym.obj\
                $(INTDIR)\flint_table.obj\
                $(INTDIR)\flint_termlist.obj\
                $(INTDIR)\flint_termlisttable.obj\
                $(INTDIR)\flint_values.obj\
                $(INTDIR)\flint_version.obj

SRCS= \
                $(INTDIR)\flint_alldocspostlist.cc\
                $(INTDIR)\flint_alltermslist.cc\
                $(INTDIR)\flint_btreebase.cc\
                $(INTDIR)\flint_compact.cc\
                $(INTDIR)\flint_cursor.cc\
                $(INTDIR)\flint_database.cc\
                $(INTDIR)\flint_databasereplicator.cc\
                $(INTDIR)\flint_document.cc\
                $(INTDIR)\flint_metadata.cc\
                $(INTDIR)\flint_modifiedpostlist.cc\
                $(INTDIR)\flint_positionlist.cc\
                $(INTDIR)\flint_postlist.cc\
                $(INTDIR)\flint_record.cc\
                $(INTDIR)\flint_spelling.cc\
                $(INTDIR)\flint_spellingwordslist.cc\
                $(INTDIR)\flint_synonym.cc\
                $(INTDIR)\flint_table.cc\
                $(INTDIR)\flint_termlist.cc\
                $(INTDIR)\flint_termlisttable.cc\
                $(INTDIR)\flint_values.cc\
                $(INTDIR)\flint_version.cc\
                $(INTDIR)\flint_check.cc

CLEAN :
    -@erase "$(OUTDIR)\libflint.lib" 
    -@erase "$(OUTDIR)\libflintbtreecheck.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase "$(INTDIR)\flint_check.obj"
    -@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)-$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I "..\.." -I "..\..\include" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" 
 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBFLINT.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libflint.lib" $(DEF_FLAGS) $(OBJS)
<<

"$(OUTDIR)\LIBFLINTBTREECHECK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBFLINTBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libflintbtreecheck.lib" $(DEF_FLAGS) $(LIBFLINTBTREECHECK_OBJS)
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