# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build and run tests

!INCLUDE ..\win32\config.mak

OUTLIBDIR= ..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
OUTDIR= ..\tests
INTDIR= ..\tests


PROGRAM_APITEST= "$(OUTDIR)\apitest.exe"
PROGRAM_INTERNALTEST= "$(OUTDIR)\internaltest.exe"
PROGRAM_QUERYPARSERTEST= "$(OUTDIR)\queryparsertest.exe"
PROGRAM_STEMTEST= "$(OUTDIR)\stemtest.exe"
PROGRAM_TERMGENTEST= "$(OUTDIR)\termgentest.exe"
PROGRAM_UNITTEST= "$(OUTDIR)\unittest.exe"

ALL : $(CLEAN_COLLATED_HEADERS) $(PROGRAM_APITEST) $(PROGRAM_INTERNALTEST) \
 $(PROGRAM_QUERYPARSERTEST) $(PROGRAM_STEMTEST) $(PROGRAM_TERMGENTEST) $(PROGRAM_UNITTEST)


APITEST : $(PROGRAM_APITEST)
STEMTEST : $(PROGRAM_STEMTEST)
INTERNALTEST : $(PROGRAM_INTERNALTEST)
QUERYPARSERTEST : $(PROGRAM_QUERYPARSERTEST)
TERMGENTEST : $(PROGRAM_TERMGENTEST)
UNITTEST : $(PROGRAM_UNITTEST)


DOTEST :
    set srcdir=.
    copy "$(ZLIB_BIN_DIR)\zlib1.dll"
    apitest -v
    internaltest
    queryparsertest
    stemtest
    termgentest
    unittest


#    remotetest
#  $(PROGRAM_REMOTETEST) not built
# REMOTETEST : $(PROGRAM_REMOTETEST)
#PROGRAM_REMOTETEST= "$(OUTDIR)\remotetest.exe"

# object files

STEMTEST_OBJS= "$(OUTDIR)\stemtest.obj"

APITEST_OBJS= \
    "$(OUTDIR)\apitest.obj" \
    "$(OUTDIR)\api_anydb.obj" \
    "$(OUTDIR)\api_backend.obj" \
    "$(OUTDIR)\api_closedb.obj" \
    "$(OUTDIR)\api_collapse.obj" \
    "$(OUTDIR)\api_compact.obj" \
    "$(OUTDIR)\api_db.obj" \
    "$(OUTDIR)\api_generated.obj" \
    "$(OUTDIR)\api_matchspy.obj" \
    "$(OUTDIR)\api_metadata.obj" \
    "$(OUTDIR)\api_nodb.obj" \
    "$(OUTDIR)\api_none.obj" \
    "$(OUTDIR)\api_opsynonym.obj" \
    "$(OUTDIR)\api_opvalue.obj" \
    "$(OUTDIR)\api_percentages.obj" \
    "$(OUTDIR)\api_posdb.obj" \
    "$(OUTDIR)\api_postingsource.obj" \
    "$(OUTDIR)\api_qpbackend.obj" \
    "$(OUTDIR)\api_query.obj" \
    "$(OUTDIR)\api_replacedoc.obj" \
    "$(OUTDIR)\api_replicate.obj" \
    "$(OUTDIR)\api_scalability.obj" \
    "$(OUTDIR)\api_serialise.obj" \
    "$(OUTDIR)\api_sorting.obj" \
    "$(OUTDIR)\api_sortingold.obj" \
    "$(OUTDIR)\api_spelling.obj" \
    "$(OUTDIR)\api_stem.obj" \
    "$(OUTDIR)\api_transdb.obj" \
    "$(OUTDIR)\api_unicode.obj" \
    "$(OUTDIR)\api_valuestats.obj" \
    "$(OUTDIR)\api_valuestream.obj" \
    "$(OUTDIR)\api_wrdb.obj" \
    "$(OUTDIR)\dbcheck.obj"

INTERNALTEST_OBJS= "$(OUTDIR)\internaltest.obj"

QUARTZTEST_OBJS= "$(OUTDIR)\quartztest.obj"

QUERYPARSERTEST_OBJS= "$(OUTDIR)\queryparsertest.obj"

REMOTETEST_OBJS= "$(OUTDIR)\remotetest.obj"

TERMGENTEST_OBJS= "$(OUTDIR)\termgentest.obj"

UNITTEST_OBJS= "$(OUTDIR)\unittest.obj"

SRC = \
    "$(INTDIR)\apitest.cc" \
    "$(INTDIR)\dbcheck.cc" \
    "$(INTDIR)\internaltest.cc" \
    "$(INTDIR)\queryparsertest.cc" \
    "$(INTDIR)\stemtest.cc" \
    "$(INTDIR)\termgentest.cc"  \
    "$(INTDIR)\unittest.cc"

COLLATED_APITEST_SOURCES= \
    "$(INTDIR)\api_anydb.cc" \
    "$(INTDIR)\api_backend.cc" \
    "$(INTDIR)\api_closedb.cc" \
    "$(INTDIR)\api_collapse.cc" \
    "$(INTDIR)\api_compact.cc" \
    "$(INTDIR)\api_db.cc" \
    "$(INTDIR)\api_generated.cc" \
    "$(INTDIR)\api_matchspy.cc" \
    "$(INTDIR)\api_metadata.cc" \
    "$(INTDIR)\api_nodb.cc" \
    "$(INTDIR)\api_none.cc" \
    "$(INTDIR)\api_opsynonym.cc" \
    "$(INTDIR)\api_opvalue.cc" \
    "$(INTDIR)\api_percentages.cc" \
    "$(INTDIR)\api_posdb.cc" \
    "$(INTDIR)\api_postingsource.cc" \
    "$(INTDIR)\api_qpbackend.cc" \
    "$(INTDIR)\api_query.cc" \
    "$(INTDIR)\api_replacedoc.cc" \
    "$(INTDIR)\api_replicate.cc" \
    "$(INTDIR)\api_scalability.cc" \
    "$(INTDIR)\api_serialise.cc" \
    "$(INTDIR)\api_sorting.cc" \
    "$(INTDIR)\api_sortingold.cc" \
    "$(INTDIR)\api_spelling.cc" \
    "$(INTDIR)\api_stem.cc" \
    "$(INTDIR)\api_transdb.cc" \
    "$(INTDIR)\api_unicode.cc" \
    "$(INTDIR)\api_valuestats.cc" \
    "$(INTDIR)\api_valuestream.cc" \
    "$(INTDIR)\api_wrdb.cc"

COLLATED_APITEST_HEADERS=\
    "$(INTDIR)\api_anydb.h" \
    "$(INTDIR)\api_backend.h" \
    "$(INTDIR)\api_closedb.h" \
    "$(INTDIR)\api_collapse.h" \
    "$(INTDIR)\api_compact.h" \
    "$(INTDIR)\api_db.h" \
    "$(INTDIR)\api_generated.h" \
    "$(INTDIR)\api_matchspy.h" \
    "$(INTDIR)\api_metadata.h" \
    "$(INTDIR)\api_nodb.h" \
    "$(INTDIR)\api_none.h" \
    "$(INTDIR)\api_opsynonym.h" \
    "$(INTDIR)\api_percentages.h" \
    "$(INTDIR)\api_posdb.h" \
    "$(INTDIR)\api_postingsource.h" \
    "$(INTDIR)\api_qpbackend.h" \
    "$(INTDIR)\api_query.h" \
    "$(INTDIR)\api_replacedoc.h" \
    "$(INTDIR)\api_replicate.h" \
    "$(INTDIR)\api_scalability.h" \
    "$(INTDIR)\api_serialise.h" \
    "$(INTDIR)\api_sorting.h" \
    "$(INTDIR)\api_sortingold.h" \
    "$(INTDIR)\api_opsynonym.h" \
    "$(INTDIR)\api_spelling.h" \
    "$(INTDIR)\api_transdb.h" \
    "$(INTDIR)\api_unicode.h" \
    "$(INTDIR)\api_valuestats.h" \
    "$(INTDIR)\api_valuestream.h" \
    "$(INTDIR)\api_wrdb.h"

CLEAN_COLLATED_HEADERS:
    -@erase api_collated.h
    -@erase $(COLLATED_APITEST_HEADERS)

CLEAN :
    -@erase $(PROGRAM_APITEST)
    -@erase $(PROGRAM_INTERNALTEST)
    -@erase $(PROGRAM_QUERYPARSERTEST)
    -@erase $(PROGRAM_REMOTETEST)
    -@erase $(PROGRAM_STEMTEST)
    -@erase $(PROGRAM_TERMGENTEST)
    -@erase $(APITEST_OBJS)
    -@erase $(INTERNALTEST_OBJS)
    -@erase $(QUERYPARSERTEST_OBJS)
    -@erase $(STEMTEST_OBJS)
    -@erase $(TERMGENTEST_OBJS)
    -@erase "$(INTDIR)\*.obj"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\*.ilk"
    -@erase "$(INTDIR)\*.exp"
    -@erase "$(INTDIR)\*.lib"
    -@erase "$(INTDIR)\*.manifest"
    -@erase api_collated.h
    -@erase api_generated.cc
    -@erase api_all.h
    -@erase $(COLLATED_APITEST_HEADERS)
    if exist ".btreetmp" rmdir ".btreetmp" /s /q
    if exist ".stub" rmdir ".stub" /s /q
    if exist ".quartz" rmdir ".quartz" /s /q
    if exist ".flint" rmdir ".flint" /s /q
    if exist ".chert" rmdir ".chert" /s /q
    if exist ".brass" rmdir ".brass" /s /q
    if exist ".multiflint" rmdir ".multiflint" /s /q
    if exist ".multichert" rmdir ".multichert" /s /q
    if exist ".multibrass" rmdir ".multibrass" /s /q
    if exist ".quartztmp" rmdir ".quartztmp" /s /q



CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I ".." -I "..\tests" -I "harness" -I"..\backends\quartz" \
 -Fo"$(INTDIR)\\" -Fd"$(INTDIR)\\" -Tp$(INPUTNAME)

CPP_OBJS=..\win32\Tests$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


api_collated.h api_all.h: collate-test $(COLLATED_APITEST_SOURCES)
    $(PERL_EXE) "$(INTDIR)/collate-test" "$(INTDIR)" api_collated.h api_all.h $(COLLATED_APITEST_SOURCES)

"$(INTDIR)/api_generated.cc": generate-api_generated
    $(PERL_EXE) "$(INTDIR)/generate-api_generated" > api_generated.cc

LINK32=link.exe
ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) "$(OUTLIBDIR)\libtest.lib"

PROGRAM_DEPENDENCIES = $(XAPIAN_LIBS) "$(OUTLIBDIR)\libtest.lib"

# executables
"$(OUTDIR)\termgentest.exe" : "$(OUTDIR)" $(DEF_FILE) $(TERMGENTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\termgentest.exe" $(DEF_FLAGS) $(TERMGENTEST_OBJS)
<<

"$(OUTDIR)\stemtest.exe" : "$(OUTDIR)" $(DEF_FILE) $(STEMTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\stemtest.exe" $(DEF_FLAGS) $(STEMTEST_OBJS)
<<
"$(OUTDIR)\internaltest.exe" : "$(OUTDIR)" $(DEF_FILE) $(INTERNALTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\internaltest.exe" $(DEF_FLAGS) $(INTERNALTEST_OBJS)
<<


"$(OUTDIR)\queryparsertest.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUERYPARSERTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\queryparsertest.exe" $(DEF_FLAGS) $(QUERYPARSERTEST_OBJS)
<<

"$(OUTDIR)\apitest.exe" : api_collated.h "$(OUTDIR)" $(DEF_FILE) $(APITEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\apitest.exe" $(DEF_FLAGS) $(APITEST_OBJS)
<<

"$(OUTDIR)\remotetest.exe" : "$(OUTDIR)" $(DEF_FILE) $(REMOTETEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\remotetest.exe" $(DEF_FLAGS) $(REMOTETEST_OBJS)
<<

"$(OUTDIR)\unittest.exe" : "$(OUTDIR)" $(DEF_FILE) $(UNITTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\unittest.exe" $(DEF_FLAGS) $(UNITTEST_OBJS)
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
HEADERS : api_collated.h
    -@erase deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(SRC) >>deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(COLLATED_APITEST_SOURCES) >>deps.d
    if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND)
# DO NOT DELETE THIS LINE -- xapdep depends on it.