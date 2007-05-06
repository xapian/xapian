# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libqueryparser.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL :  "$(INTDIR)\lemon.exe" "$(OUTDIR)\libqueryparser.lib"

LIBQUERYPARSER_OBJS= \
                $(INTDIR)\queryparser.obj \
                $(INTDIR)\queryparser_internal.obj \
                $(INTDIR)\termgenerator.obj \
                $(INTDIR)\termgenerator_internal.obj


CLEAN :
	-@erase "$(OUTDIR)\libqueryparser.lib"
	-@erase "*.pch"
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

"$(INTDIR)\termgenerator.obj" : ".\termgenerator.cc"
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

"$(INTDIR)\termgenerator_internal.obj" : ".\termgenerator_internal.cc"
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

