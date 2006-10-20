# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libflint.lib


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\..\win32\config.mak


CPP=cl.exe
RSC=rc.exe


OUTDIR=..\..\win32\Release\libs
INTDIR=.\

ALL : "$(OUTDIR)\libflint.lib" 

LIBFLINT_OBJS= \
               $(INTDIR)\flint_database.obj \
               $(INTDIR)\flint_termlist.obj \
               $(INTDIR)\flint_postlist.obj \
               $(INTDIR)\flint_modifiedpostlist.obj \
               $(INTDIR)\flint_positionlist.obj \
               $(INTDIR)\flint_record.obj \
               $(INTDIR)\flint_values.obj \
               $(INTDIR)\flint_document.obj \
               $(INTDIR)\flint_alldocspostlist.obj \
               $(INTDIR)\flint_alltermslist.obj \
               $(INTDIR)\flint_table.obj \
               $(INTDIR)\flint_cursor.obj \
               $(INTDIR)\flint_btreebase.obj \
               $(INTDIR)\flint_version.obj \
               $(INTDIR)\flint_io.obj \
               $(INTDIR)\flint_lock.obj

CLEAN :
	-@erase "$(OUTDIR)\libflint.lib"
	-@erase "*.pch"
    -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBFLINT_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) /W3 /GX /O2 \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /Fo"$(INTDIR)\\" \
 /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"
CPP_OBJS=..\..\win32\Release
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)


"$(OUTDIR)\LIBFLINT.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBFLINT_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libflint.lib" $(DEF_FLAGS) $(LIBFLINT_OBJS)
<<



"$(INTDIR)\flint_database.obj" : "flint_database.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_termlist.obj" : "flint_termlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_postlist.obj" : "flint_postlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_modifiedpostlist.obj" : "flint_modifiedpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_positionlist.obj" : "flint_positionlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_record.obj" : "flint_record.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_values.obj" : "flint_values.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_document.obj" : "flint_document.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_alldocspostlist.obj" : "flint_alldocspostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_alltermslist.obj" : "flint_alltermslist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_table.obj" : "flint_table.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_cursor.obj" : "flint_cursor.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_btreebase.obj" : "flint_btreebase.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_version.obj" : "flint_version.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_io.obj" : "flint_io.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\flint_lock.obj" : "flint_lock.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<




{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

