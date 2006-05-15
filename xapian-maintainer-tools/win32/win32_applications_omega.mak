# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 31st March 2006

# Will build the following  programs
# scriptindex.exe
# omega.exe
# omindex.exe

XAPIAN_DIR=..\xapian-core-0.9.6


!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE $(XAPIAN_DIR)\win32\config.mak


CPP=cl.exe
RSC=rc.exe


OUTDIR=.
INTDIR=.
OUTLIBDIR=$(XAPIAN_DIR)\win32\Release\libs
OUTEXEDIR=$(XAPIAN_DIR)\win32\Release

PROGRAMS =   "$(OUTEXEDIR)\scriptindex.exe" "$(OUTEXEDIR)\omindex.exe" "$(OUTEXEDIR)\omega.exe" 

#	copy $(XAPIAN_DIR)\win32\config.h.win32 ..\config.h 

ALL : $(PROGRAMS)

OMEGA_OBJS= \
	"$(OUTDIR)\omega.obj" \
	"$(OUTDIR)\query.obj" \
	"$(OUTDIR)\cgiparam.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\configfile.obj" \
	"$(OUTDIR)\date.obj" \
	"$(OUTDIR)\cdb_init.obj" \
	"$(OUTDIR)\cdb_find.obj" \
	"$(OUTDIR)\cdb_hash.obj" \
	"$(OUTDIR)\cdb_unpack.obj" 

OMINDEX_OBJS= \
	"$(OUTDIR)\omindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\indextext.obj" \
	"$(OUTDIR)\getopt.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj" \
	"$(OUTDIR)\dirent.obj"

SCRIPTINDEX_OBJS= \
	"$(OUTDIR)\scriptindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\indextext.obj" \
	"$(OUTDIR)\getopt.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj"


CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(OMEGA_OBJS)
	-@erase $(OMINDEX_OBJS)
	-@erase $(SCRIPTINDEX_OBJS)

#"$(OUTDIR)" :
#    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) /W3 /GX /O2 \
 /I "." /I ".." /I "$(XAPIAN_DIR)" /I "$(XAPIAN_DIR)\common" /I "$(XAPIAN_DIR)\include" /I "$(XAPIAN_DIR)\win32" \
 /DCONFIGFILE_SYSTEM=NULL \
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /YX \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"

CPP_OBJS=..\win32\Release
CPP_SBRS=.

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console \
 "$(OUTLIBDIR)\libgetopt.lib"  \
 "$(OUTLIBDIR)\libcommon.lib"  \
 "$(OUTLIBDIR)\libbtreecheck.lib"  \
 "$(OUTLIBDIR)\libtest.lib"  \
 "$(OUTLIBDIR)\libbackend.lib"  \
 "$(OUTLIBDIR)\libquartz.lib" \
 "$(OUTLIBDIR)\libflint.lib" \
 "$(OUTLIBDIR)\libinmemory.lib" \
 "$(OUTLIBDIR)\libmulti.lib" \
 "$(OUTLIBDIR)\libmatcher.lib"  \
 "$(OUTLIBDIR)\liblanguages.lib"  \
 "$(OUTLIBDIR)\libapi.lib"  \
 "$(OUTLIBDIR)\libqueryparser.lib"  

 

PROGRAM_DEPENDENCIES = 

 # omega.exe
# omindex.exe
# scriptindex.exe

"$(OUTEXEDIR)\omega.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(OMEGA_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTEXEDIR)\omega.exe" $(DEF_FLAGS) $(OMEGA_OBJS)
<<

"$(OUTEXEDIR)\omindex.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(OMINDEX_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTEXEDIR)\omindex.exe" $(DEF_FLAGS) $(OMINDEX_OBJS)
<<

"$(OUTEXEDIR)\scriptindex.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(SCRIPTINDEX_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTEXEDIR)\scriptindex.exe" $(DEF_FLAGS) $(SCRIPTINDEX_OBJS)
<<

"$(INTDIR)\dirent.obj" : "$(XAPIAN_DIR)\win32\dirent.c"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<
 
 
"$(INTDIR)\omega.obj" : ".\omega.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\query.obj" : ".\query.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\cgiparam.obj" : ".\cgiparam.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\utils.obj" : ".\utils.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\configfile.obj" : ".\configfile.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\date.obj" : ".\date.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\cdb_init.obj" : ".\cdb_init.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\cdb_find.obj" : ".\cdb_find.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\cdb_hash.obj" : ".\cdb_hash.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\cdb_unpack.obj" : ".\cdb_unpack.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\omindex.obj" : ".\omindex.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\scriptindex.obj" : ".\scriptindex.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\myhtmlparse.obj" : ".\myhtmlparse.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\htmlparse.obj" : ".\htmlparse.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\indextext.obj" : ".\indextext.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\getopt.obj" : ".\getopt.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\commonhelp.obj" : ".\commonhelp.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\utils.obj" : ".\utils.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\hashterm.obj" : ".\hashterm.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

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
