# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 19th February 2006

# Will build a Win32 executable snowball.exe

!INCLUDE ..\..\win32\config.mak

OUTDIR =..\compiler
INTDIR=..\compiler

ALL : "$(OUTDIR)\snowball.exe"

OBJS= 	       "$(INTDIR)\analyser.obj" \
               "$(INTDIR)\generator.obj" \
               "$(INTDIR)\driver.obj" \
               "$(INTDIR)\space.obj" \
               "$(INTDIR)\tokeniser.obj" 

SRCS= 	       "$(INTDIR)\analyser.c" \
               "$(INTDIR)\generator.c" \
               "$(INTDIR)\driver.c" \
               "$(INTDIR)\space.c" \
               "$(INTDIR)\tokeniser.c" 

CLEAN :
	-@erase "$(INTDIR)\*.pch"
        -@erase $(OBJS)
	-@erase "$(INTDIR)\*.pdb"
	-@erase "$(OUTDIR)\*.exe"


CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -D "DISABLE_JAVA" \
 -I ".." -I "..\compiler" \
 -Fo"$(INTDIR)\\" -Fd"$(INTDIR)\\" -Tc"$(INPUTNAME)" 
CPP_OBJS=..\compiler
CPP_SBRS=.


"$(OUTDIR)\snowball.exe" : "$(OUTDIR)" $(DEF_FILE) $(OBJS) 
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\snowball.exe" $(DEF_FLAGS) $(OBJS)
<<

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

# Calculate any header dependencies and automatically insert them into this file
HEADERS :
    -@erase deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(SRCS) >>deps.d
    if exist "..\..\win32\$(DEPEND)" ..\..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.