# Makefile for Microsoft Visual C++ 7.0 (or compatible)
#  by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th June 2008

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\tests
OUTLIBDIR= ..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

BUILD_ALL = "$(OUTDIR)\perftest.exe"

ALL : $(BUILD_ALL) get_machine_info

OBJS= \
        "$(INTDIR)\freemem.obj" \
        "$(INTDIR)\perftest.obj" \
        "$(INTDIR)\runprocess.obj" \
        "$(INTDIR)\perftest_matchdecider.obj" \
        "$(INTDIR)\perftest_randomidx.obj"

SRCS= \
        "$(INTDIR)\freemem.cc" \
        "$(INTDIR)\perftest.cc" \
        "$(INTDIR)\runprocess.cc" \
        "$(INTDIR)\perftest_matchdecider.cc" \
        "$(INTDIR)\perftest_randomidx.cc"

COLLATED_PERFTEST_SOURCES=perftest_matchdecider.cc perftest_randomidx.cc
    
COLLATED_PERFTEST_HEADERS="$(INTDIR)\perftest_randomidx.h" "$(INTDIR)\perftest_matchdecider.h"         

CLEAN :
        -@erase $(BUILD_ALL)
        -@erase "*.pch"
        -@erase "$(INTDIR)\*.pdb"
        -@erase $(OBJS)
        -@erase perftest_collated.h
        -@erase perftest_all.h
        -@erase $(COLLATED_PERFTEST_HEADERS)        
        
perftest_all.h: perftest_collated.h
    
perftest_collated.h: ..\collate-test $(COLLATED_PERFTEST_SOURCES)
    $(PERL_EXE) "..\collate-test" "$(INTDIR)" perftest_collated.h perftest_all.h $(COLLATED_PERFTEST_SOURCES) 
    
get_machine_info: get_machine_info.in
    copy get_machine_info.in get_machine_info
    

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\.." -I"..\..\include" -I"..\..\api" -I"..\..\common" -I"..\..\languages" -I"..\harness"\
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) "$(OUTLIBDIR)\libtest.lib" 
 
# executables
$(BUILD_ALL) : perftest_all.h $(OUTDIR) $(DEF_FILE) $(OBJS) $(XAPIAN_LIBS) "$(OUTLIBDIR)\libtest.lib"
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(BUILD_ALL)" $(DEF_FLAGS) $(OBJS)
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