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
 	"$(OUTDIR)\loadfile.obj" \
 	"$(OUTDIR)\utf8convert.obj" \
 	"$(OUTDIR)\datematchdecider.obj" 
	
OMINDEX_OBJS= \
	"$(OUTDIR)\omindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\getopt.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj" \
 	"$(OUTDIR)\loadfile.obj" \
 	"$(OUTDIR)\md5.obj" \
 	"$(OUTDIR)\md5wrap.obj" \
 	"$(OUTDIR)\xmlparse.obj" \
 	"$(OUTDIR)\metaxmlparse.obj" \
 	"$(OUTDIR)\utf8convert.obj" \
	"$(OUTDIR)\sample.obj" \
	"$(OUTDIR)\mkdtemp.obj" \
	"$(OUTDIR)\dirent.obj" 
	
	
SCRIPTINDEX_OBJS= \
	"$(OUTDIR)\scriptindex.obj" \
	"$(OUTDIR)\myhtmlparse.obj" \
	"$(OUTDIR)\htmlparse.obj" \
	"$(OUTDIR)\getopt.obj" \
	"$(OUTDIR)\commonhelp.obj" \
	"$(OUTDIR)\utils.obj" \
	"$(OUTDIR)\hashterm.obj" \
	"$(OUTDIR)\loadfile.obj" \
	"$(OUTDIR)\safe.obj" \
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
	
LOCAL_HEADERS= omega.h query.h cgiparam.h\
 myhtmlparse.h htmlparse.h utils.h configfile.h date.h\
 commonhelp.h cdb.h cdb_int.h hashterm.h loadfile.h\
 md5.h md5wrap.h xmlparse.h metaxmlparse.h values.h utf8convert.h\
 namedentities.h datematchdecider.h sample.h strcasecmp.h\
 utf8truncate.h diritor.h \
# headers maintained in xapian-core
	common/gnu_getopt.h\
	common/safeerrno.h\
	common/safefcntl.h\
	common/safesysselect.h\
	common/safesysstat.h\
	common/safeunistd.h\
	common/safewindows.h
	

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

# if any headers change, rebuild all .objs
$(OMEGA_OBJS): $(LOCAL_HEADERS)
$(OMINDEX_OBJS): $(LOCAL_HEADERS)
$(SCRIPTINDEX_OBJS): $(LOCAL_HEADERS)
$(HTMLPARSETEST_OBJS): $(LOCAL_HEADERS)
$(MD5TEST_OBJS): $(LOCAL_HEADERS) 
 
"$(INTDIR)\getopt.obj" : ".\common\getopt.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\mkdtemp.obj" : ".\portability\mkdtemp.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\safe.obj" : ".\common\safe.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

# inference rules, showing how to create one type of file from another with the same root name
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
