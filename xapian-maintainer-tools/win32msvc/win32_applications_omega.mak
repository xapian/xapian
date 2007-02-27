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
OUTLIBDIR=$(XAPIAN_CORE_REL_OMEGA)\win32\Release\libs
OUTEXEDIR=$(XAPIAN_CORE_REL_OMEGA)\win32\Release

PROGRAMS =   "$(OUTEXEDIR)\scriptindex.exe" "$(OUTEXEDIR)\omindex.exe" "$(OUTEXEDIR)\omega.exe" \
"$(OUTEXEDIR)\md5test.exe" "$(OUTEXEDIR)\htmlparsetest.exe" "$(OUTEXEDIR)\utf8test.exe" 

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
 	"$(OUTDIR)\utf8itor.obj" \
 	"$(OUTDIR)\datematchdecider.obj" 
  

OMINDEX_OBJS= \
	"$(OUTDIR)\omindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\indextext.obj" \
	"$(OUTDIR)\getopt.obj" \
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
 	"$(OUTDIR)\utf8itor.obj" \
 	"$(OUTDIR)\tclUniData.obj" 
  

SCRIPTINDEX_OBJS= \
	"$(OUTDIR)\scriptindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\indextext.obj" \
	"$(OUTDIR)\getopt.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj" \
	"$(OUTDIR)\loadfile.obj" \
	"$(OUTDIR)\utf8convert.obj" \
 	"$(OUTDIR)\utf8itor.obj" 
	
HTMLPARSETEST_OBJS= \
 	"$(OUTDIR)\htmlparsetest.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
 	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\utf8convert.obj" \
 	"$(OUTDIR)\utf8itor.obj" 

MD5TEST_OBJS= \
 	"$(OUTDIR)\md5.obj" \
 	"$(OUTDIR)\md5wrap.obj" \
 	"$(OUTDIR)\md5test.obj" 
	
UTF8TEST_OBJS= \
 	"$(OUTDIR)\utf8test.obj" \
 	"$(OUTDIR)\utf8itor.obj"
	

CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(OMEGA_OBJS)
	-@erase $(OMINDEX_OBJS)
	-@erase $(SCRIPTINDEX_OBJS)
	-@erase $(HTMLPARSETEST_OBJS)
	-@erase $(MD5TEST_OBJS)
	-@erase $(UTF8TEST_OBJS)

#"$(OUTDIR)" :
#    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I "." /I "$(XAPIAN_CORE_REL_OMEGA)" /I "$(XAPIAN_CORE_REL_OMEGA)\common" /I "$(XAPIAN_CORE_REL_OMEGA)\include" /I "$(XAPIAN_CORE_REL_OMEGA)\win32" \
 /DCONFIGFILE_SYSTEM=NULL \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tp$(INPUTNAME) 

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

"$(OUTEXEDIR)\htmlparsetest.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(HTMLPARSETEST_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTEXEDIR)\htmlparsetest.exe" $(DEF_FLAGS) $(HTMLPARSETEST_OBJS)
<<

"$(OUTEXEDIR)\md5test.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(MD5TEST_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTEXEDIR)\md5test.exe" $(DEF_FLAGS) $(MD5TEST_OBJS)
<<

"$(OUTEXEDIR)\utf8test.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(UTF8TEST_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTEXEDIR)\utf8test.exe" $(DEF_FLAGS) $(UTF8TEST_OBJS)
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

"$(INTDIR)\utf8itor.obj" : ".\utf8itor.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\utf8convert.obj" : ".\utf8convert.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\tclUniData.obj" : ".\tclUniData.cc"
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
