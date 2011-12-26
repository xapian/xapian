# NMAKE file for xapdep tool

OUTDIR=.
INTDIR=.

ALL : "$(OUTDIR)\xapdep.exe"

CLEAN :
	-@erase "$(INTDIR)\xapdep.obj"
	-@erase "$(OUTDIR)\xapdep.exe"
    -@erase "$(OUTDIR)\*.idb

CPP=cl.exe
CPP_PROJ=/nologo /W3 /EHsc /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D STANDALONE=1 /D OBJSUFFIX=\".obj\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\xapdep.pdb" /machine:I386 /out:"$(OUTDIR)\xapdep.exe"
LINK32_OBJS= "$(INTDIR)\xapdep.obj"

"$(OUTDIR)\xapdep.exe" : "$(OUTDIR)" $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

