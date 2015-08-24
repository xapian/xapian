# Makefile for Microsoft Visual C++ 7.0 (or compatible)
#  by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com

# Will build a Win32 static library (non-debug) libbrass.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libbrass.lib"  "$(OUTDIR)\libbrassbtreecheck.lib" 

LIBBRASSBTREECHECK_OBJS= \
                $(INTDIR)\brass_check.obj

OBJS= \
                $(INTDIR)\brass_alldocspostlist.obj\
                $(INTDIR)\brass_alltermslist.obj\
                $(INTDIR)\brass_btreebase.obj\
                $(INTDIR)\brass_compact.obj\
                $(INTDIR)\brass_cursor.obj\
                $(INTDIR)\brass_database.obj\
                $(INTDIR)\brass_databasereplicator.obj\
                $(INTDIR)\brass_dbstats.obj\
                $(INTDIR)\brass_document.obj\
                $(INTDIR)\brass_inverter.obj\
                $(INTDIR)\brass_metadata.obj\
                $(INTDIR)\brass_positionlist.obj\
                $(INTDIR)\brass_postlist.obj\
                $(INTDIR)\brass_record.obj\
                $(INTDIR)\brass_spelling.obj\
                $(INTDIR)\brass_spellingwordslist.obj\
                $(INTDIR)\brass_synonym.obj\
                $(INTDIR)\brass_table.obj\
                $(INTDIR)\brass_termlist.obj\
                $(INTDIR)\brass_termlisttable.obj\
                $(INTDIR)\brass_values.obj\
                $(INTDIR)\brass_valuelist.obj\
                $(INTDIR)\brass_version.obj

SRCS= \
                $(INTDIR)\brass_alldocspostlist.cc\
                $(INTDIR)\brass_alltermslist.cc\
                $(INTDIR)\brass_btreebase.cc\
                $(INTDIR)\brass_compact.cc\
                $(INTDIR)\brass_cursor.cc\
                $(INTDIR)\brass_database.cc\
                $(INTDIR)\brass_databasereplicator.cc\
                $(INTDIR)\brass_dbstats.cc\
                $(INTDIR)\brass_document.cc\
                $(INTDIR)\brass_inverter.cc\
                $(INTDIR)\brass_metadata.cc\
                $(INTDIR)\brass_positionlist.cc\
                $(INTDIR)\brass_postlist.cc\
                $(INTDIR)\brass_record.cc\
                $(INTDIR)\brass_spelling.cc\
                $(INTDIR)\brass_spellingwordslist.cc\
                $(INTDIR)\brass_synonym.cc\
                $(INTDIR)\brass_table.cc\
                $(INTDIR)\brass_termlist.cc\
                $(INTDIR)\brass_termlisttable.cc\
                $(INTDIR)\brass_values.cc\
                $(INTDIR)\brass_valuelist.cc\
                $(INTDIR)\brass_version.cc\
                $(INTDIR)\brass_check.cc

CLEAN :
    -@erase "$(OUTDIR)\libbrass.lib" 
    -@erase "$(OUTDIR)\libbrassbtreecheck.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\getopt.obj"
    -@erase "$(INTDIR)\brass_check.obj"
    -@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)-$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I "..\.." -I "..\..\include" -I"..\..\common" -I"..\..\languages" \
 -Fo"$(INTDIR)\\" 
 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\libbrass.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libbrass.lib" $(DEF_FLAGS) $(OBJS)
<<

"$(OUTDIR)\libbrassbtreecheck.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBBRASSBTREECHECK_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) -out:"$(OUTDIR)\libbrassbtreecheck.lib" $(DEF_FLAGS) $(LIBBRASSBTREECHECK_OBJS)
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
            if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) $(DEPEND_FLAGS) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)" 
# DO NOT DELETE THIS LINE -- make depend depends on it.

