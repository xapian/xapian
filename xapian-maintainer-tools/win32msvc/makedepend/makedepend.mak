# Microsoft Developer Studio Generated NMAKE File, Based on makedepend.dsp

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\makedepend.exe"

CLEAN :
	-@erase "$(INTDIR)\cppsetup.obj"
	-@erase "$(INTDIR)\ifparser.obj"
	-@erase "$(INTDIR)\include.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\parse.obj"
	-@erase "$(INTDIR)\pr.obj"
	-@erase "$(INTDIR)\vc80.idb"
	-@erase "$(OUTDIR)\makedepend.exe"

CPP=cl.exe
CPP_PROJ=/nologo /W3 /EHsc /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D STANDALONE=1 /D OBJSUFFIX=\".obj\" /Fp"$(INTDIR)\makedepend.pch"  /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\makedepend.pdb" /machine:I386 /out:"$(OUTDIR)\makedepend.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cppsetup.obj" \
	"$(INTDIR)\ifparser.obj" \
	"$(INTDIR)\include.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\pr.obj"

"$(OUTDIR)\makedepend.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
# Microsoft Developer Studio Generated NMAKE File, Based on makedepend.dsp

OUTDIR=.
INTDIR=.
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\makedepend.exe"

CLEAN :
	-@erase "$(INTDIR)\cppsetup.obj"
	-@erase "$(INTDIR)\ifparser.obj"
	-@erase "$(INTDIR)\include.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\parse.obj"
	-@erase "$(INTDIR)\pr.obj"
	-@erase "$(INTDIR)\vc80.idb"
	-@erase "$(OUTDIR)\makedepend.exe"

CPP=cl.exe
CPP_PROJ=/nologo /W3 /EHsc /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D STANDALONE=1 /D OBJSUFFIX=\".obj\" /Fp"$(INTDIR)\makedepend.pch"  /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\makedepend.pdb" /machine:I386 /out:"$(OUTDIR)\makedepend.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cppsetup.obj" \
	"$(INTDIR)\ifparser.obj" \
	"$(INTDIR)\include.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\pr.obj"

"$(OUTDIR)\makedepend.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<
