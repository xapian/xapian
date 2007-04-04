# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libqueryparser.lib


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

ALL :  "$(INTDIR)\lemon.exe" "$(OUTDIR)\libqueryparser.lib"

LIBQUERYPARSER_OBJS= \
                $(INTDIR)\queryparser.obj \
                $(INTDIR)\queryparser_internal.obj 


CLEAN :
	-@erase "$(OUTDIR)\libqueryparser.lib"
	-@erase "*.pch"
        -@erase $(LIBQUERYPARSER_OBJS)
	-@erase $(LIBQUERYPARSER_OBJS)
	-@erase queryparser_internal.cc
	-@erase lemon.exe
	-@erase lemon.obj


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\api" /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_PROJ_LEMON=$(CPPFLAGS_EXTRA) \
 /Fo"$(INTDIR)\\" /Tc$(INPUTNAME)

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)

"$(OUTDIR)\LIBQUERYPARSER.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBQUERYPARSER_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libqueryparser.lib" $(DEF_FLAGS) $(LIBQUERYPARSER_OBJS)
<<

"$(INTDIR)\lemon.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\lemon.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(INTDIR)\lemon.exe" $(DEF_FLAGS) "$(INTDIR)\lemon.obj"
<<

"$(INTDIR)\lemon.obj" : ".\lemon.c"
    $(CPP) ".\lemon.c" @<<
  $(CPP_PROJ_LEMON) $**
<<

"$(INTDIR)\queryparser.obj" : ".\queryparser.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\queryparser_internal.obj" : ".\queryparser_internal.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

"$(INTDIR)\queryparser_internal.cc" : ".\queryparser.lemony" 
	$(INTDIR)\lemon.exe -q -oqueryparser_internal.cc -hqueryparser_token.h queryparser.lemony 
	
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

