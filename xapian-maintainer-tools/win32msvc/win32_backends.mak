# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libbackend.lib


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\win32\config.mak

CPP=cl.exe
RSC=rc.exe


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

DEPLIBS = "$(OUTDIR)\libinmemory.lib" \
          "$(OUTDIR)\libmulti.lib"  \
		  "$(OUTDIR)\libquartz.lib" \
	      "$(OUTDIR)\libflint.lib" 
#		  "$(OUTDIR)\libmuscat36.lib"

ALL : $(DEPLIBS) "$(OUTDIR)\libbackend.lib" 

CLEAN :
	-@erase "$(OUTDIR)\libbackend.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\database.obj"
	-@erase $(CLEANFILES)
	cd quartz
	nmake /f win32.mak CLEAN
	cd ..\flint
	nmake /f win32.mak CLEAN
	cd ..\inmemory
	nmake /f win32.mak CLEAN
	cd ..\multi
	nmake /f win32.mak CLEAN
#	cd ..\muscat36
# nmake /f win32.mak CLEAN
	cd ..


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)
LIBBACKEND_OBJS= $(INTDIR)\database.obj 


"$(OUTDIR)\LIBBACKEND.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBBACKEND_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libbackend.lib" $(DEF_FLAGS) $(LIBBACKEND_OBJS)
<<

"$(INTDIR)\database.obj" : ".\database.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

"$(OUTDIR)\libquartz.lib":
       cd quartz
       nmake /f win32.mak 
       cd ..

"$(OUTDIR)\libflint.lib":
       cd flint
       nmake /f win32.mak 
       cd ..

"$(OUTDIR)\libinmemory.lib":
       cd inmemory
       nmake /f win32.mak 
       cd ..

"$(OUTDIR)\libmulti.lib":
       cd multi
       nmake /f win32.mak 
       cd ..

# "$(OUTDIR)\libmuscat36.lib":
# cd muscat36
#nmake /f win32.mak 
#       cd ..
