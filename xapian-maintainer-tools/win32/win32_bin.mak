# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build quartzcheck.exe, quartzcompact.exe, quartdump.exe and xapian_compact.exe


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\win32\config.mak


CPP=cl.exe
RSC=rc.exe

OUTDIR=..\win32\Release
OUTLIBDIR=..\win32\Release\libs
INTDIR=.\

PROGRAMS = "$(OUTDIR)\quartzcheck.exe" "$(OUTDIR)\quartzcompact.exe" "$(OUTDIR)\quartzdump.exe" "$(OUTDIR)\xapian-compact.exe" 

ALL : $(PROGRAMS)

QUARTZCHECK_OBJS= \
	"$(INTDIR)\quartzcheck.obj" 

QUARTZCOMPACT_OBJS= \
	"$(INTDIR)\quartzcompact.obj" 

QUARTZDUMP_OBJS= \
	"$(INTDIR)\quartzdump.obj" 

XAPIAN_COMPACT_OBJS= \
	"$(INTDIR)\xapian-compact.obj" 

	
CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(QUARTZCHECK_OBJS)
	-@erase $(QUARTZCOMPACT_OBJS)
	-@erase $(QUARTZDUMP_OBJS)
	-@erase $(XAPIAN_COMPACT_OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I ".." /I "..\testsuite" /I"..\backends\quartz" /I"..\backends\flint" \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tp$(INPUTNAME)

CPP_OBJS=..\win32\Release
CPP_SBRS=.

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console \
 "$(OUTLIBDIR)\libgetopt.lib"  \
 "$(OUTLIBDIR)\libcommon.lib"  \
 "$(OUTLIBDIR)\libbtreecheck.lib"  \
 "$(OUTLIBDIR)\libtest.lib"  \
 "$(OUTLIBDIR)\libbackend.lib"  \
 "$(OUTLIBDIR)\libquartz.lib" \
 "$(OUTLIBDIR)\libflint.lib" \
 "$(OUTLIBDIR)\libinmemory.lib" \
 "$(OUTLIBDIR)\libmulti.lib" \
 "$(OUTLIBDIR)\libmatcher.lib"  \
 "$(OUTLIBDIR)\liblanguages.lib"  \
 "$(OUTLIBDIR)\libapi.lib"  \
 "$(OUTLIBDIR)\libqueryparser.lib"  

 
PROGRAM_DEPENDENCIES = 








"$(OUTDIR)\quartzcheck.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZCHECK_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\quartzcheck.exe" $(DEF_FLAGS) $(QUARTZCHECK_OBJS)
<<


"$(OUTDIR)\quartzcompact.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZCOMPACT_OBJS) \
                                 $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\quartzcompact.exe" $(DEF_FLAGS) $(QUARTZCOMPACT_OBJS)
<<

"$(OUTDIR)\quartzdump.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZDUMP_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\quartzdump.exe" $(DEF_FLAGS) $(QUARTZDUMP_OBJS)
<<

"$(OUTDIR)\xapian-compact.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_COMPACT_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\xapian-compact.exe" $(DEF_FLAGS) $(XAPIAN_COMPACT_OBJS)
<<

    
"$(INTDIR)\quartzcompact.obj" : ".\quartzcompact.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartzdump.obj" : ".\quartzdump.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\quartzcheck.obj" : ".\quartzcheck.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\xapian-compact.obj" : ".\xapian-compact.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
