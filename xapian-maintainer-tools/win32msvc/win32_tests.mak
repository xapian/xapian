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
PROGRAM_BTREETEST= "$(OUTDIR)\btreetest.exe" 
PROGRAM_INTERNALTEST= "$(OUTDIR)\internaltest.exe" 
PROGRAM_QUARTZTEST= "$(OUTDIR)\quartztest.exe" 
PROGRAM_QUERYPARSERTEST= "$(OUTDIR)\queryparsertest.exe"
PROGRAM_STEMTEST= "$(OUTDIR)\stemtest.exe"
PROGRAM_TERMGENTEST= "$(OUTDIR)\termgentest.exe"

ALL : $(PROGRAM_APITEST) $(PROGRAM_BTREETEST) $(PROGRAM_INTERNALTEST) \
 $(PROGRAM_QUARTZTEST) $(PROGRAM_QUERYPARSERTEST) $(PROGRAM_STEMTEST) $(PROGRAM_TERMGENTEST)
 
 
APITEST : $(PROGRAM_APITEST) 
STEMTEST : $(PROGRAM_STEMTEST)  
BTREETEST : $(PROGRAM_BTREETEST)  
INTERNALTEST : $(PROGRAM_INTERNALTEST)  
QUARTZTEST : $(PROGRAM_QUARTZTEST)  
QUERYPARSERTEST : $(PROGRAM_QUERYPARSERTEST)  
TERMGENTEST : $(PROGRAM_TERMGENTEST)  


DOTEST :
	set srcdir=.
	copy "$(ZLIB_BIN_DIR)\zlib1.dll"
	apitest -v
	btreetest
	internaltest
	quartztest
	queryparsertest
	stemtest
	termgentest

	
#	remotetest
#  $(PROGRAM_REMOTETEST) not built
# REMOTETEST : $(PROGRAM_REMOTETEST)  
#PROGRAM_REMOTETEST= "$(OUTDIR)\remotetest.exe" 

# object files
 
STEMTEST_OBJS= "$(OUTDIR)\stemtest.obj" 

APITEST_OBJS= \
	"$(OUTDIR)\apitest.obj" \
	"$(OUTDIR)\api_anydb.obj" \
	"$(OUTDIR)\api_db.obj" \
	"$(OUTDIR)\api_nodb.obj" \
	"$(OUTDIR)\api_posdb.obj" \
        "$(OUTDIR)\api_transdb.obj" \
        "$(OUTDIR)\api_unicode.obj" \
	"$(OUTDIR)\api_wrdb.obj" 
    
BTREETEST_OBJS= "$(OUTDIR)\btreetest.obj"

INTERNALTEST_OBJS= "$(OUTDIR)\internaltest.obj"
	
QUARTZTEST_OBJS= "$(OUTDIR)\quartztest.obj"

QUERYPARSERTEST_OBJS= "$(OUTDIR)\queryparsertest.obj"
	
REMOTETEST_OBJS= "$(OUTDIR)\remotetest.obj"	

TERMGENTEST_OBJS= "$(OUTDIR)\termgentest.obj"	

LOCAL_HEADERS = "$(INTDIR)\apitest.h"

CLEAN :
	-@erase $(PROGRAM_APITEST) 
	-@erase $(PROGRAM_BTREETEST)
	-@erase $(PROGRAM_INTERNALTEST) 
 	-@erase $(PROGRAM_QUARTZTEST) 
	-@erase $(PROGRAM_QUERYPARSERTEST) 
	-@erase $(PROGRAM_REMOTETEST)
	-@erase $(PROGRAM_STEMTEST)
	-@erase $(PROGRAM_TERMGENTEST)
	-@erase $(APITEST_OBJS)
	-@erase $(BTREETEST_OBJS)
	-@erase $(INTERNALTEST_OBJS)
	-@erase $(QUARTZTEST_OBJS)
	-@erase $(QUERYPARSERTEST_OBJS)
	-@erase $(STEMTEST_OBJS)
	-@erase $(TERMGENTEST_OBJS)
	if exist ".btreetmp" rmdir ".btreetmp" /s /q
	if exist ".flint" rmdir ".flint" /s /q
	if exist ".quartz" rmdir ".quartz" /s /q
	if exist ".quartztmp" rmdir ".quartztmp" /s /q
	
CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I ".." /I "..\tests" /I "harness" /I"..\backends\quartz" \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tp$(INPUTNAME) 

CPP_OBJS=..\win32\Tests$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.



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

"$(OUTDIR)\btreetest.exe" : "$(OUTDIR)" $(DEF_FILE) $(BTREETEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\btreetest.exe" $(DEF_FLAGS) $(BTREETEST_OBJS)
<<

"$(OUTDIR)\internaltest.exe" : "$(OUTDIR)" $(DEF_FILE) $(INTERNALTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\internaltest.exe" $(DEF_FLAGS) $(INTERNALTEST_OBJS)
<<

"$(OUTDIR)\quartztest.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\quartztest.exe" $(DEF_FLAGS) $(QUARTZTEST_OBJS)
<<

"$(OUTDIR)\queryparsertest.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUERYPARSERTEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\queryparsertest.exe" $(DEF_FLAGS) $(QUERYPARSERTEST_OBJS)
<<

"$(OUTDIR)\apitest.exe" : "$(OUTDIR)" $(DEF_FILE) $(APITEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\apitest.exe" $(DEF_FLAGS) $(APITEST_OBJS)
<<

"$(OUTDIR)\remotetest.exe" : "$(OUTDIR)" $(DEF_FILE) $(REMOTETEST_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\remotetest.exe" $(DEF_FLAGS) $(REMOTETEST_OBJS)
<<

# if any headers change, rebuild all .objs
$(APITEST_OBJS): $(LOCAL_HEADERS)
$(BTREETEST_OBJS): $(LOCAL_HEADERS)
$(INTERNALTEST_OBJS): $(LOCAL_HEADERS)
$(QUARTZTEST_OBJS): $(LOCAL_HEADERS)
$(QUERYPARSERTEST_OBJS): $(LOCAL_HEADERS)
$(REMOTETEST_OBJS): $(LOCAL_HEADERS)
$(TERMGENTEST_OBJS): $(LOCAL_HEADERS)


# inference rules, showing how to create one type of file from another with the same root name	
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
