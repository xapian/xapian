# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libbackend.lib

!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

DEPLIBS = "$(OUTDIR)\libinmemory.lib" \
          "$(OUTDIR)\libmulti.lib"  \
          "$(OUTDIR)\libquartz.lib" \
          "$(OUTDIR)\libremote.lib" \
          "$(OUTDIR)\libflint.lib" \
          $(NULL)

ALL : $(DEPLIBS) "$(OUTDIR)\libbackend.lib" 

CLEAN :
	-@erase "$(OUTDIR)\libbackend.lib"
	-@erase "$(INTDIR)\*.pch"
	-@erase "$(INTDIR)\*.pdb"
	-@erase $(LIBBACKEND_OBJS)
	cd quartz
	nmake /$(MAKEFLAGS) CLEAN DEBUG=$(DEBUG) 
	cd ..\flint
	nmake /$(MAKEFLAGS) CLEAN DEBUG=$(DEBUG) 
	cd ..\inmemory
	nmake /$(MAKEFLAGS) CLEAN DEBUG=$(DEBUG) 
	cd ..\multi
	nmake /$(MAKEFLAGS) CLEAN DEBUG=$(DEBUG) 
	cd ..\remote
	nmake /$(MAKEFLAGS) CLEAN DEBUG=$(DEBUG) 
	cd ..


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIBBACKEND_OBJS= $(INTDIR)\database.obj $(INTDIR)\dbfactory_remote.obj $(INTDIR)\alltermslist.obj 

"$(OUTDIR)\LIBBACKEND.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBBACKEND_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libbackend.lib" $(DEF_FLAGS) $(LIBBACKEND_OBJS)
<<

"$(OUTDIR)\libquartz.lib":
       cd quartz
       nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
       cd ..

"$(OUTDIR)\libflint.lib":
       cd flint
       nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
       cd ..

"$(OUTDIR)\libinmemory.lib":
       cd inmemory
       nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
       cd ..

"$(OUTDIR)\libmulti.lib":
       cd multi
       nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
       cd ..

"$(OUTDIR)\libremote.lib":
       cd remote
       nmake $(MAKEMACRO) /$(MAKEFLAGS) CFG="$(CFG)" DEBUG="$(DEBUG)"
       cd ..

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

