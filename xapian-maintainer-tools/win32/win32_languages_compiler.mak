# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 19th February 2006

# Will build a Win32 executable snowball.exe


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\..\win32\config.mak

CPP=cl.exe
RSC=rc.exe

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

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console \


"$(OUTDIR)\snowball.exe" : "$(OUTDIR)" $(DEF_FILE) $(SNOWBALL_OBJS) \
                      $(PROGRAM_DEPENDENCIES)
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

