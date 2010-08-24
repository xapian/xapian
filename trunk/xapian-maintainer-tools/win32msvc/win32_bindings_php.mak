# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 28th Feb 2007

# Will build the PHP bindings 

# Where the core is, relative to the PHP bindings
# Change this to match your environment
XAPIAN_CORE_REL_PHP=..\..\xapian-core

OUTLIBDIR=$(XAPIAN_CORE_REL_PHP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs

!INCLUDE $(XAPIAN_CORE_REL_PHP)\win32\config.mak

LIB_XAPIAN_OBJS= ".\xapian_wrap.obj" ".\version.res" 

CPP=cl.exe
RSC=rc.exe

OUTROOT=$(XAPIAN_CORE_REL_PHP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\PHP
OUTDIR=$(OUTROOT)\php$(PHP_MAJOR_VERSION)
INTDIR=.\

	
ALL : "$(OUTDIR)\php_xapian.dll" "$(OUTDIR)\xapian.php" \
	"$(OUTROOT)\smoketest.php" "$(OUTDIR)\smoketest$(PHP_MAJOR_VERSION).php" 

CLEAN :
	-@erase "$(OUTDIR)\php_xapian.dll"
	-@erase "$(OUTDIR)\php_xapian.exp"
	-@erase "$(OUTDIR)\php_xapian.lib"
	-@erase $(LIB_XAPIAN_OBJS)
	-@erase "$(OUTDIR)\xapian.php"
	-@erase "$(OUTDIR)\smoketest4.php"
	-@erase "$(OUTDIR)\smoketest5.php"
	-@erase "$(OUTROOT)\smoketest.php"

	
CLEANSWIG :	
	-@erase /Q /s php4
	-@erase /Q /s php5
	if exist "php4" rmdir "php4" /s /q
	if exist "php5" rmdir "php5" /s /q

DOTEST :
	cd "$(OUTROOT)"
	$(PHP_EXE) -q -n -d safe_mode=off -d enable_dl=on -d extension_dir="php$(PHP_MAJOR_VERSION)" -d include_path="php$(PHP_MAJOR_VERSION)" smoketest.php

CHECK: ALL DOTEST

DIST: CHECK 
    cd $(MAKEDIR)
    if not exist "$(OUTDIR)\dist/$(NULL)" mkdir "$(OUTDIR)\dist"
    if not exist "$(OUTDIR)\dist\docs/$(NULL)" mkdir "$(OUTDIR)\dist\docs"
    if not exist "$(OUTDIR)\dist\docs\examples/$(NULL)" mkdir "$(OUTDIR)\dist\docs\examples"    
    copy "$(OUTDIR)\php_xapian.dll" "$(OUTDIR)\dist"
    copy "$(OUTDIR)\xapian.php" "$(OUTDIR)\dist"
    copy docs\*.html "$(OUTDIR)\dist\docs"
    copy docs\examples\*.* "$(OUTDIR)\dist\docs\examples"
	
"$(OUTROOT)" :
    if not exist "$(OUTROOT)/$(NULL)" mkdir "$(OUTROOT)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
    
    
CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_PHP)" /I "$(XAPIAN_CORE_REL_PHP)\include" $(PHP_INCLUDE_CPPFLAGS) $(PHP_DEBUG_OR_RELEASE)\
 /I"." /Fo"$(INTDIR)\\" /Tp$(INPUTNAME) 
CPP_OBJS=$(XAPIAN_CORE_REL_PHP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\
CPP_SBRS=.

!IF "$(SWIGBUILD)" == "1"
php4/xapian_wrap.cc php4/php_xapian.h php4/xapian.php: ../xapian.i util.i
	-erase /Q /s php4
	-md php4
	$(SWIG) -I"$(XAPIAN_CORE_REL_PHP)\include" $(SWIG_FLAGS) -c++ -php4 \
	    -outdir php4 -o php4/xapian_wrap.cc $(srcdir)/../xapian.i

php5/xapian_wrap.cc php5/php_xapian.h php5/xapian.php: ../xapian.i util.i
	-erase /Q /s php5
	-md php5
	$(SWIG) -I"$(XAPIAN_CORE_REL_PHP)\include" $(SWIG_FLAGS) -c++ -php5 -prefix Xapian \
	    -outdir php5 -o php5/xapian_wrap.cc $(srcdir)/../xapian.i
!ENDIF

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) $(PHP_LIB)


"$(OUTDIR)\php_xapian.dll" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) 
                            
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\php_xapian.dll" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<

"$(OUTDIR)\xapian.php" : php$(PHP_MAJOR_VERSION)\xapian.php
	-copy $** "$(OUTDIR)\xapian.php"
# REMOVE THIS NEXT LINE if using Visual C++ .net 2003 - you won't need to worry about manifests
	$(MANIFEST) "$(OUTDIR)\php_xapian.dll.manifest" -outputresource:"$(OUTDIR)\php_xapian.dll;2"
	-@erase "$(OUTDIR)\php_xapian.dll.manifest"
"$(OUTDIR)\smoketest5.php" : ".\smoketest5.php"
	-copy $** "$(OUTDIR)\smoketest5.php"
"$(OUTDIR)\smoketest4.php" : ".\smoketest4.php"
	-copy $** "$(OUTDIR)\smoketest4.php"
"$(OUTROOT)\smoketest.php" : ".\smoketest.php"
	-copy $** "$(OUTROOT)\smoketest.php"
#
# Rules
#

".\version.res": version.rc
    $(RSC) /v \
      /fo version.res \
      /I "$(XAPIAN_CORE_REL_PHP)\include" \
      /I "$(PHP_SRC_DIR)\main" \
      /d PHP_MAJOR_VERSION="\"$(PHP_MAJOR_VERSION)\"" \
      version.rc 

".\xapian_wrap.obj" : "php$(PHP_MAJOR_VERSION)\xapian_wrap.cc"
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

