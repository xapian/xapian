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
"$(OUTEXEDIR)\md5test.exe" "$(OUTEXEDIR)\htmlparsetest.exe" "$(OUTEXEDIR)\utf8converttest.exe" 

ALL : HEADERS $(PROGRAMS) 

OMEGA_OBJS= \
 omindex.obj myhtmlparse.obj htmlparse.obj\
 getopt.obj commonhelp.obj utils.obj hashterm.obj loadfile.obj md5.obj\
 md5wrap.obj xmlparse.obj metaxmlparse.obj utf8convert.obj sample.obj diritor.obj\
 runfilter.obj freemem.obj msvc_dirent.obj xpsxmlparse.obj mkdtemp.obj
	
OMINDEX_OBJS= \
 omindex.obj myhtmlparse.obj htmlparse.obj\
 getopt.obj commonhelp.obj utils.obj hashterm.obj loadfile.obj md5.obj\
 md5wrap.obj xmlparse.obj metaxmlparse.obj utf8convert.obj sample.obj diritor.obj\
 runfilter.obj freemem.obj msvc_dirent.obj xpsxmlparse.obj mkdtemp.obj
	
	
SCRIPTINDEX_OBJS= \
 scriptindex.obj myhtmlparse.obj htmlparse.obj\
 getopt.obj commonhelp.obj utils.obj hashterm.obj loadfile.obj\
 safe.obj stringutils.obj utf8convert.obj utf8truncate.obj
	
	
HTMLPARSETEST_OBJS= \
htmlparsetest.obj myhtmlparse.obj htmlparse.obj utf8convert.obj

MD5TEST_OBJS= \
md5test.obj md5wrap.obj md5.obj

UTF8CONVERTTEST_OBJS= \
utf8converttest.obj utf8convert.obj

SRCS= \
	"$(INTDIR)\omega.cc" \
	"$(INTDIR)\query.cc" \
	"$(INTDIR)\cgiparam.cc" \
	"$(INTDIR)\utils.cc" \
	"$(INTDIR)\configfile.cc" \
	"$(INTDIR)\date.cc" \
	"$(INTDIR)\cdb_init.cc" \
	"$(INTDIR)\cdb_find.cc" \
	"$(INTDIR)\cdb_hash.cc" \
	"$(INTDIR)\cdb_unpack.cc" \
 	"$(INTDIR)\loadfile.cc" \
 	"$(INTDIR)\utf8convert.cc" \
 	"$(INTDIR)\datematchdecider.cc" \
	"$(INTDIR)\omindex.cc" \
	"$(INTDIR)\myhtmlparse.cc" \
	"$(INTDIR)\htmlparse.cc" \
	"$(INTDIR)\common\getopt.cc" \
	"$(INTDIR)\commonhelp.cc" \
	"$(INTDIR)\utils.cc" \
	"$(INTDIR)\hashterm.cc" \
 	"$(INTDIR)\loadfile.cc" \
 	"$(INTDIR)\md5.cc" \
 	"$(INTDIR)\md5wrap.cc" \
 	"$(INTDIR)\xmlparse.cc" \
 	"$(INTDIR)\metaxmlparse.cc" \
 	"$(INTDIR)\utf8convert.cc" \
	"$(INTDIR)\sample.cc" \
	"$(INTDIR)\portability\mkdtemp.cc" \
	"$(INTDIR)\scriptindex.cc" \
	"$(INTDIR)\myhtmlparse.cc" \
	"$(INTDIR)\htmlparse.cc" \
	"$(INTDIR)\common\getopt.cc" \
	"$(INTDIR)\commonhelp.cc" \
	"$(INTDIR)\utils.cc" \
	"$(INTDIR)\hashterm.cc" \
	"$(INTDIR)\loadfile.cc" \
	"$(INTDIR)\common\safe.cc" \
	"$(INTDIR)\utf8converttest.cc" \
	"$(INTDIR)\utf8truncate.cc" \
 	"$(INTDIR)\htmlparsetest.cc" \
	"$(INTDIR)\myhtmlparse.cc" \
 	"$(INTDIR)\htmlparse.cc" \
	"$(INTDIR)\utf8convert.cc" \
 	"$(INTDIR)\md5.cc" \
 	"$(INTDIR)\md5wrap.cc" \
 	"$(INTDIR)\md5test.cc" \
	"$(INTDIR)\diritor.cc" \
	"$(INTDIR)\runfilter.cc" \
    "$(OUTDIR)\xpsxmlparse.cc" \
    "$(OUTDIR)\weight.cc" \
    "$(OUTDIR)\freemem.cc"
    
	
CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(OMEGA_OBJS)
	-@erase $(OMINDEX_OBJS)
	-@erase $(SCRIPTINDEX_OBJS)
	-@erase $(HTMLPARSETEST_OBJS)
    -@erase $(UTF8CONVERTTEST_OBJS)
	-@erase $(MD5TEST_OBJS)
	-@erase "$(INTDIR)\*.pdb"


CPP_PROJ=$(CPPFLAGS_EXTRA) \
-I.. -I. -I "common" -I "$(XAPIAN_CORE_REL_OMEGA)\include" -I "$(XAPIAN_CORE_REL_OMEGA)\win32" \
 -Fo"$(INTDIR)\\" -Fd"$(INTDIR)\\" -Tp$(INPUTNAME) 

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

"$(OUTEXEDIR)\UTF8CONVERTTEST.exe" : "$(OUTEXEDIR)" $(DEF_FILE) $(UTF8CONVERTTEST_OBJS)
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTEXEDIR)\UTF8CONVERTTEST.exe" $(DEF_FLAGS) $(UTF8CONVERTTEST_OBJS)
<<

# rules for various files in subfolders

"dirent.obj" : "$(XAPIAN_CORE_REL_OMEGA)\win32\dirent.c"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"getopt.obj" : ".\common\getopt.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"stringutils.obj" : ".\common\stringutils.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"mkdtemp.obj" : ".\portability\mkdtemp.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"safe.obj" : ".\common\safe.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"msvc_dirent.obj" : "$(XAPIAN_CORE_REL_OMEGA)\common\msvc_dirent.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

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
    if exist "$(XAPIAN_CORE_REL_OMEGA)\win32\$(DEPEND)" $(XAPIAN_CORE_REL_OMEGA)\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.