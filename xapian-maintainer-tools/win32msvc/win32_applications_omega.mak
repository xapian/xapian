# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 31st March 2006

# Will build the following  programs
# scriptindex.exe
# omega.exe
# omindex.exe

# Where the core is, relative to the Omega application
# Change this to match your environment
XAPIAN_CORE_REL_OMEGA=..\..\xapian-core

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE $(XAPIAN_CORE_REL_OMEGA)\win32\config.mak


CPP=cl.exe
RSC=rc.exe


OUTDIR=.
INTDIR=.
OUTLIBDIR=$(XAPIAN_CORE_REL_OMEGA)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
OUTEXEDIR=$(XAPIAN_CORE_REL_OMEGA)\win32\$(XAPIAN_DEBUG_OR_RELEASE)

PROGRAMS =   "$(OUTEXEDIR)\scriptindex.exe" "$(OUTEXEDIR)\omindex.exe" "$(OUTEXEDIR)\omega.exe" \
"$(OUTEXEDIR)\md5test.exe" "$(OUTEXEDIR)\htmlparsetest.exe"

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
	"$(OUTDIR)\cdb_unpack.obj" \
 	"$(OUTDIR)\indextext.obj" \
 	"$(OUTDIR)\loadfile.obj" \
 	"$(OUTDIR)\utf8convert.obj" \
 	"$(OUTDIR)\datematchdecider.obj" 
  

OMINDEX_OBJS= \
	"$(OUTDIR)\omindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\indextext.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj" \
	"$(OUTDIR)\dirent.obj" \
 	"$(OUTDIR)\loadfile.obj" \
 	"$(OUTDIR)\md5.obj" \
 	"$(OUTDIR)\md5wrap.obj" \
 	"$(OUTDIR)\xmlparse.obj" \
 	"$(OUTDIR)\metaxmlparse.obj" \
 	"$(OUTDIR)\utf8convert.obj" \
	"$(OUTDIR)\mkdtemp.obj" \
	"$(OUTDIR)\sample.obj"

  

SCRIPTINDEX_OBJS= \
	"$(OUTDIR)\scriptindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\indextext.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj" \
	"$(OUTDIR)\loadfile.obj" \
	"$(OUTDIR)\utf8convert.obj" \
	"$(OUTDIR)\utf8truncate.obj" 
	
HTMLPARSETEST_OBJS= \
 	"$(OUTDIR)\htmlparsetest.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
 	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\utf8convert.obj" 

MD5TEST_OBJS= \
 	"$(OUTDIR)\md5.obj" \
 	"$(OUTDIR)\md5wrap.obj" \
 	"$(OUTDIR)\md5test.obj" 
	

CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(OMEGA_OBJS)
	-@erase $(OMINDEX_OBJS)
	-@erase $(SCRIPTINDEX_OBJS)
	-@erase $(HTMLPARSETEST_OBJS)
	-@erase $(MD5TEST_OBJS)

#"$(OUTDIR)" :
#    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I "." /I "common" /I "$(XAPIAN_CORE_REL_OMEGA)\include" /I "$(XAPIAN_CORE_REL_OMEGA)\win32" \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tp$(INPUTNAME) 

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS)

PROGRAM_DEPENDENCIES = 

 # omega.exe
# omindex.exe
# scriptindex.exe

"$(OUTEXEDIR)\omega.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(OMEGA_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTEXEDIR)\omega.exe" $(DEF_FLAGS) $(OMEGA_OBJS)
<<

"$(OUTEXEDIR)\omindex.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(OMINDEX_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTEXEDIR)\omindex.exe" $(DEF_FLAGS) $(OMINDEX_OBJS)
<<

"$(OUTEXEDIR)\scriptindex.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(SCRIPTINDEX_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTEXEDIR)\scriptindex.exe" $(DEF_FLAGS) $(SCRIPTINDEX_OBJS)
<<

"$(OUTEXEDIR)\htmlparsetest.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(HTMLPARSETEST_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTEXEDIR)\htmlparsetest.exe" $(DEF_FLAGS) $(HTMLPARSETEST_OBJS)
<<

"$(OUTEXEDIR)\md5test.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(MD5TEST_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTEXEDIR)\md5test.exe" $(DEF_FLAGS) $(MD5TEST_OBJS)
<<

"$(INTDIR)\dirent.obj" : "$(XAPIAN_CORE_REL_OMEGA)\win32\dirent.c"
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

"$(INTDIR)\hashterm.obj" : ".\hashterm.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\loadfile.obj" : ".\loadfile.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\md5.obj" : ".\md5.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\md5wrap.obj" : ".\md5wrap.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\xmlparse.obj" : ".\xmlparse.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\metaxmlparse.obj" : ".\metaxmlparse.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\datematchdecider.obj" : ".\datematchdecider.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\utf8truncate.obj" : ".\utf8truncate.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\utf8convert.obj" : ".\utf8convert.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\md5test.obj" : ".\md5test.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\htmlparsetest.obj" : ".\htmlparsetest.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\htmlparse.obj" : ".\htmlparse.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\myhtmlparse.obj" : ".\myhtmlparse.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\utf8test.obj" : ".\utf8test.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\mkdtemp.obj" : ".\portability\mkdtemp.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\sample.obj" : ".\sample.cc"
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
