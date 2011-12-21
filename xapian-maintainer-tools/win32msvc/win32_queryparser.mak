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

OBJS= \
                $(INTDIR)\queryparser.obj \
                $(INTDIR)\queryparser_internal.obj \
                $(INTDIR)\termgenerator.obj \
                $(INTDIR)\termgenerator_internal.obj

SRCS= \
                $(INTDIR)\queryparser.cc \
                $(INTDIR)\queryparser_internal.cc \
                $(INTDIR)\termgenerator.cc \
                $(INTDIR)\termgenerator_internal.cc

CLEAN :
	-@erase "$(OUTDIR)\libqueryparser.lib"
	-@erase "*.pch"
	-@erase $(OBJS)
	-@erase "$(INTDIR)\*.pdb"
	-@erase queryparser_internal.cc
	-@erase lemon.exe
	-@erase lemon.obj


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\api" -I"..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)

CPP_PROJ_LEMON=$(CPPFLAGS_EXTRA) \
 -Fo"$(INTDIR)\\" -Tc$(INPUTNAME)

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBQUERYPARSER.lib" : "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libqueryparser.lib" $(DEF_FLAGS) $(OBJS)
<<

"$(INTDIR)\lemon.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\lemon.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(INTDIR)\lemon.exe" $(DEF_FLAGS) "$(INTDIR)\lemon.obj"
<<

"$(INTDIR)\lemon.obj" : ".\lemon.c"
    $(CPP) @<<
  $(CPP_PROJ_LEMON) $** ".\lemon.c"
<<

"$(INTDIR)\queryparser_internal.cc" : ".\queryparser.lemony"
	$(INTDIR)\lemon.exe -q -oqueryparser_internal.cc -hqueryparser_token.h queryparser.lemony

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
HEADERS : "$(INTDIR)\lemon.exe" "$(INTDIR)\queryparser_internal.cc"
    -@erase deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(SRCS) >>deps.d
    if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND)
# DO NOT DELETE THIS LINE -- xapdep depends on it.