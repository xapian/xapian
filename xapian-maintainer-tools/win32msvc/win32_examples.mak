# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 31st March 2006

# Will build the following example programs
# copydatabase.exe
# delve.exe
# quest.exe
# simpleexpand.exe
# simpleindex.exe
# simplesearch.exe


!INCLUDE ..\win32\config.mak

OUTLIBDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs\

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\

INTDIR=.\

OBJS =  	$(INTDIR)\delve.obj \
		$(INTDIR)\quest.obj \
		$(INTDIR)\simpleexpand.obj \
		$(INTDIR)\simpleindex.obj \
		$(INTDIR)\simplesearch.obj \
		$(INTDIR)\copydatabase.obj
		
SRCS =  	$(INTDIR)\delve.cc \
		$(INTDIR)\quest.cc \
		$(INTDIR)\simpleexpand.cc \
		$(INTDIR)\simpleindex.cc \
		$(INTDIR)\simplesearch.cc \
		$(INTDIR)\copydatabase.cc

PROGRAMS = "$(OUTDIR)\delve.exe" "$(OUTDIR)\quest.exe" \
"$(OUTDIR)\simpleexpand.exe" "$(OUTDIR)\simpleindex.exe" "$(OUTDIR)\simplesearch.exe" "$(OUTDIR)\copydatabase.exe" 

ALL : $(PROGRAMS)

CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(OBJS)
	-@erase "$(INTDIR)\*.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -Fo"$(INTDIR)\\" -Fd"$(INTDIR)\\" -Tp$(INPUTNAME)

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


ALL_LINK32_FLAGS = $(LINK32_FLAGS) $(XAPIAN_LIBS)

PROGRAM_DEPENDENCIES = $(XAPIAN_LIBS)

 
# delve.exe
# quest.exe
# simpleexpand.exe
# simpleindex.exe
# simplesearch.exe

"$(OUTDIR)\copydatabase.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\copydatabase.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\copydatabase.exe" $(DEF_FLAGS) "$(INTDIR)\copydatabase.obj"
<<
    $(MANIFEST) "$(OUTDIR)\copydatabase.exe.manifest" -outputresource:"$(OUTDIR)\copydatabase.exe;1"
    -@erase "$(OUTDIR)\copydatabase.exe.manifest"


"$(OUTDIR)\delve.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\delve.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\delve.exe" $(DEF_FLAGS) "$(INTDIR)\delve.obj"
<<
    $(MANIFEST) "$(OUTDIR)\delve.exe.manifest" -outputresource:"$(OUTDIR)\delve.exe;1"
    -@erase "$(OUTDIR)\delve.exe.manifest"

"$(OUTDIR)\quest.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\quest.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\quest.exe" $(DEF_FLAGS) "$(INTDIR)\quest.obj"
<<
    $(MANIFEST) "$(OUTDIR)\quest.exe.manifest" -outputresource:"$(OUTDIR)\quest.exe;1"
    -@erase "$(OUTDIR)\quest.exe.manifest"


"$(OUTDIR)\simpleexpand.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\simpleexpand.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\simpleexpand.exe" $(DEF_FLAGS) "$(INTDIR)\simpleexpand.obj"
<<
    $(MANIFEST) "$(OUTDIR)\simpleexpand.exe.manifest" -outputresource:"$(OUTDIR)\simpleexpand.exe;1"
    -@erase "$(OUTDIR)\simpleexpand.exe.manifest"

"$(OUTDIR)\simpleindex.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\simpleindex.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\simpleindex.exe" $(DEF_FLAGS) "$(INTDIR)\simpleindex.obj"
<<
    $(MANIFEST) "$(OUTDIR)\simpleindex.exe.manifest" -outputresource:"$(OUTDIR)\simpleindex.exe;1"
    -@erase "$(OUTDIR)\simpleindex.exe.manifest"

"$(OUTDIR)\simplesearch.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\simplesearch.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\simplesearch.exe" $(DEF_FLAGS) "$(INTDIR)\simplesearch.obj"
<<
    $(MANIFEST) "$(OUTDIR)\simplesearch.exe.manifest" -outputresource:"$(OUTDIR)\simplesearch.exe;1"
    -@erase "$(OUTDIR)\simplesearch.exe.manifest"


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
    -@erase deps.d
    $(CPP) -showIncludes $(CPP_PROJ) $(SRCS) >>deps.d
    if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.