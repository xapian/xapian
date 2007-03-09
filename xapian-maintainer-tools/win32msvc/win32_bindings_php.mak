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

XAPIAN_DEPENDENCIES = \
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
 "$(OUTLIBDIR)\libqueryparser.lib" \
 $(PHP_LIB)

LIB_XAPIAN_OBJS= ".\xapian_wrap.obj" 

OUTROOT=$(XAPIAN_CORE_REL_PHP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\PHP
OUTDIR=$(OUTROOT)\php$(PHP_MAJOR_VERSION)
INTDIR=.\

	
ALL : "$(OUTDIR)\php_xapian.dll" "$(OUTDIR)\xapian.php" "$(OUTDIR)\smoketest$(PHP_MAJOR_VERSION).php" $(DOMANIFEST) "$(OUTDIR)\php.ini"

CLEAN :
	-@erase "$(OUTDIR)\php_xapian.dll"
	-@erase "$(OUTDIR)\php_xapian.exp"
	-@erase "$(OUTDIR)\php_xapian.lib"
	-@erase $(LIB_XAPIAN_OBJS)
	-@erase "$(OUTDIR)\xapian.php"
	-@erase "$(OUTDIR)\smoketest4.php"
	-@erase "$(OUTDIR)\smoketest5.php"
	-@erase "$(OUTDIR)\php.ini"
	
CLEANSWIG :	
	-@erase /Q /s php4
	-@erase /Q /s php5
	if exist "php4" rmdir "php4" /s /q
	if exist "php5" rmdir "php5" /s /q
	
DOTEST :
	cd "$(OUTDIR)"
	$(PHP_EXE) -q -n -d safe_mode=off -d enable_dl=on "smoketest$(PHP_MAJOR_VERSION).php"

	
"$(OUTROOT)" :	
    if not exist "$(OUTROOT)/$(NULL)" mkdir "$(OUTROOT)"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_PHP)" /I "$(XAPIAN_CORE_REL_PHP)\include" $(PHP_INCLUDE_CPPFLAGS) $(PHP_DEBUG_OR_RELEASE)\
 /I"." /Fo"$(INTDIR)\\" /Tp$(INPUTNAME) 
CPP_OBJS=$(XAPIAN_CORE_REL_PHP)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_DEPENDENCIES)


php4/xapian_wrap.cc php4/php_xapian.h php4/xapian.php: ../xapian.i util.i except.i
	-erase /Q /s php4
	-md php4
	$(SWIG) -I"$(XAPIAN_CORE_REL_PHP)\include" $(SWIG_FLAGS) -c++ -php4 \
	    -outdir php4 -o php4/xapian_wrap.cc $(srcdir)/../xapian.i

php5/xapian_wrap.cc php5/php_xapian.h php5/xapian.php: ../xapian.i util.i except.i
	-erase /Q /s php5
	-md php5
	$(SWIG) -I"$(XAPIAN_CORE_REL_PHP)\include" $(SWIG_FLAGS) -c++ -php5 -prefix Xapian \
	    -outdir php5 -o php5/xapian_wrap.cc $(srcdir)/../xapian.i


"$(OUTDIR)\php_xapian.dll" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) \
                            $(XAPIAN_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\php_xapian.dll" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<

"$(OUTDIR)\xapian.php" : "php$(PHP_MAJOR_VERSION)\xapian.php"
	-copy $** "$(OUTDIR)\xapian.php"
	$(MANIFEST) "$(OUTDIR)\php_xapian.dll.manifest" -outputresource:"$(OUTDIR)\php_xapian.dll;2"
"$(OUTDIR)\smoketest5.php" : ".\smoketest5.php"
	-copy $** "$(OUTDIR)\smoketest5.php"
"$(OUTDIR)\smoketest4.php" : ".\smoketest4.php"
	-copy $** "$(OUTDIR)\smoketest4.php"
"$(OUTDIR)\php.ini" : ".\php.ini"
	-copy $** "$(OUTDIR)\php.ini"

#
# Rules
#

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
