# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 19th February 2006

# Will build a Win32 executable snowball.exe

!INCLUDE ..\..\win32\config.mak

OUTDIR =..\compiler
INTDIR=..\compiler

ALL : "$(OUTDIR)\snowball.exe"

SNOWBALL_OBJS= "$(INTDIR)\analyser.obj" \
               "$(INTDIR)\generator.obj" \
               "$(INTDIR)\driver.obj" \
               "$(INTDIR)\space.obj" \
               "$(INTDIR)\tokeniser.obj" 

SNOWBALL_HEADERS =\
	"$(INTDIR)\header.h" \
	"$(INTDIR)\syswords.h" \
	"$(INTDIR)\syswords2.h"

CLEAN :
	-@erase "$(INTDIR)\*.pch"
        -@erase $(SNOWBALL_OBJS)
	-@erase "$(OUTDIR)\*.exe"


CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /D "DISABLE_JAVA" \
 /I ".." /I "..\compiler" \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tc"$(INPUTNAME)" 
CPP_OBJS=..\compiler
CPP_SBRS=.


"$(OUTDIR)\snowball.exe" : "$(OUTDIR)" $(DEF_FILE) $(SNOWBALL_OBJS) 
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\snowball.exe" $(DEF_FLAGS) $(SNOWBALL_OBJS)
<<

# if any headers change, rebuild all .objs
$(SNOWBALL_OBJS): $(SNOWBALL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
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
