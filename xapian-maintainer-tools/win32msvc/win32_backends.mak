# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libbackend.lib

!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

DEPLIBS = "$(OUTDIR)\libmulti.lib"  \
	  "$(OUTDIR)\libinmemory.lib" \
          "$(OUTDIR)\libquartz.lib" \
          "$(OUTDIR)\libremote.lib" \
          "$(OUTDIR)\libflint.lib" \
          $(NULL)

OBJS= $(INTDIR)\database.obj $(INTDIR)\dbfactory_remote.obj $(INTDIR)\alltermslist.obj 
SRCS= $(INTDIR)\database.cc $(INTDIR)\dbfactory_remote.cc $(INTDIR)\alltermslist.cc 
	  
ALL : $(DEPLIBS) "$(OUTDIR)\libbackend.lib" 

CLEAN :
	-@erase "$(OUTDIR)\libbackend.lib"
	-@erase "$(INTDIR)\*.pch"
	-@erase "$(INTDIR)\*.pdb"
	-@erase $(OBJS)
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
 -I"..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBBACKEND.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libbackend.lib" $(DEF_FLAGS) $(OBJS)
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
            ..\win32\$(DEPEND) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)"
# DO NOT DELETE THIS LINE -- make depend depends on it.
