# Makefile for Microsoft Visual C++ 7.0 (or compatible)
#  by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com

# Will build a Win32 static library (non-debug) libchert.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libchert.lib"  "$(OUTDIR)\libchertbtreecheck.lib" 

LIBCHERTBTREECHECK_OBJS= \
                $(INTDIR)\chert_check.obj

OBJS= \
                $(INTDIR)\chert_alldocsmodifiedpostlist.obj\
                $(INTDIR)\chert_alldocspostlist.obj\
                $(INTDIR)\chert_alltermslist.obj\
                $(INTDIR)\chert_btreebase.obj\
                $(INTDIR)\chert_compact.obj\
                $(INTDIR)\chert_cursor.obj\
                $(INTDIR)\chert_database.obj\
                $(INTDIR)\chert_databasereplicator.obj\
                $(INTDIR)\chert_dbstats.obj\
                $(INTDIR)\chert_document.obj\
                $(INTDIR)\chert_metadata.obj\
                $(INTDIR)\chert_modifiedpostlist.obj\
                $(INTDIR)\chert_positionlist.obj\
                $(INTDIR)\chert_postlist.obj\
                $(INTDIR)\chert_record.obj\
                $(INTDIR)\chert_spelling.obj\
                $(INTDIR)\chert_spellingwordslist.obj\
                $(INTDIR)\chert_synonym.obj\
                $(INTDIR)\chert_table.obj\
                $(INTDIR)\chert_termlist.obj\
                $(INTDIR)\chert_termlisttable.obj\
                $(INTDIR)\chert_values.obj\
                $(INTDIR)\chert_valuelist.obj\
                $(INTDIR)\chert_version.obj

SRCS= \
                $(INTDIR)\chert_alldocsmodifiedpostlist.cc\
                $(INTDIR)\chert_alldocspostlist.cc\
                $(INTDIR)\chert_alltermslist.cc\
                $(INTDIR)\chert_btreebase.cc\
                $(INTDIR)\brass_compact.cc\
                $(INTDIR)\chert_cursor.cc\
                $(INTDIR)\chert_database.cc\
                $(INTDIR)\chert_databasereplicator.cc\
                $(INTDIR)\chert_dbstats.cc\
                $(INTDIR)\chert_document.cc\
                $(INTDIR)\chert_metadata.cc\
                $(INTDIR)\chert_modifiedpostlist.cc\
                $(INTDIR)\chert_positionlist.cc\
                $(INTDIR)\chert_postlist.cc\
                $(INTDIR)\chert_record.cc\
                $(INTDIR)\chert_spelling.cc\
                $(INTDIR)\chert_spellingwordslist.cc\
                $(INTDIR)\chert_synonym.cc\
                $(INTDIR)\chert_table.cc\
                $(INTDIR)\chert_termlist.cc\
                $(INTDIR)\chert_termlisttable.cc\
                $(INTDIR)\chert_values.cc\
                $(INTDIR)\chert_valuelist.cc\
                $(INTDIR)\chert_version.cc\
                $(INTDIR)\chert_check.cc

CLEAN :
    -@erase "$(OUTDIR)\libchert.lib" 
    -@erase "$(OUTDIR)\libchertbtreecheck.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase "$(INTDIR)\chert_check.obj"
    -@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)-$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I "..\.." -I "..\..\include" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" 
 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBCHERT.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libchert.lib" $(DEF_FLAGS) $(OBJS)
<<

"$(OUTDIR)\LIBCHERTBTREECHECK.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBCHERTBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libchertbtreecheck.lib" $(DEF_FLAGS) $(LIBCHERTBTREECHECK_OBJS)
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