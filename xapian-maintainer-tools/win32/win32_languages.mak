# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) liblanguages.lib


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


ALL : "$(OUTDIR)\liblanguages.lib" 

LIBLANGUAGES_OBJS= \
                 $(INTDIR)\api.obj \
                 $(INTDIR)\utilities.obj \
                 $(INTDIR)\snowball_danish.obj \
                 $(INTDIR)\snowball_dutch.obj \
                 $(INTDIR)\snowball_english.obj \
                 $(INTDIR)\snowball_french.obj \
                 $(INTDIR)\snowball_german.obj \
		 $(INTDIR)\snowball_finnish.obj \
                 $(INTDIR)\snowball_italian.obj \
		 $(INTDIR)\snowball_lovins.obj \
                 $(INTDIR)\snowball_norwegian.obj \
                 $(INTDIR)\snowball_porter.obj \
                 $(INTDIR)\snowball_portuguese.obj \
                 $(INTDIR)\snowball_russian.obj \
                 $(INTDIR)\snowball_spanish.obj \
                 $(INTDIR)\snowball_swedish.obj \

CLEAN :
	-@erase "$(OUTDIR)\liblanguages.lib"
	-@erase "*.pch"
        -@erase $(LIBLANGUAGES_OBJS)
	


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)

"$(OUTDIR)\LIBLANGUAGES.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBLANGUAGES_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\liblanguages.lib" $(DEF_FLAGS) $(LIBLANGUAGES_OBJS)
<<



"$(INTDIR)\api.obj" : ".\api.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_danish.obj" : ".\snowball_danish.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_dutch.obj" : ".\snowball_dutch.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_english.obj" : ".\snowball_english.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_finnish.obj" : ".\snowball_finnish.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_french.obj" : ".\snowball_french.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_german.obj" : ".\snowball_german.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_italian.obj" : ".\snowball_italian.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_lovins.obj" : ".\snowball_lovins.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_norwegian.obj" : ".\snowball_norwegian.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_porter.obj" : ".\snowball_porter.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_portuguese.obj" : ".\snowball_portuguese.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_russian.obj" : ".\snowball_russian.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_spanish.obj" : ".\snowball_spanish.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\snowball_swedish.obj" : ".\snowball_swedish.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\utilities.obj" : ".\utilities.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<
