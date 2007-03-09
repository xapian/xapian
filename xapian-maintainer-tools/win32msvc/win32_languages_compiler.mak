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

# Snowball compiler sources


"$(INTDIR)\driver.obj" : "driver.c"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

"$(INTDIR)\tokeniser.obj" : "tokeniser.c"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

"$(INTDIR)\space.obj" : "space.c"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

"$(INTDIR)\analyser.obj" : "analyser.c"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

"$(INTDIR)\generator.obj" : "generator.c"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

