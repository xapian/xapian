# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd. www.lemurconsulting.com
# 17th March 2006
# Copyright (C) 2007, Olly Betts

# Will build the Python bindings 

# Where the core is, relative to the Python bindings
# Change this to match your environment

XAPIAN_CORE_REL_PYTHON=..\..\xapian-core
OUTLIBDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs

!INCLUDE $(XAPIAN_CORE_REL_PYTHON)\win32\config.mak

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
 "$(OUTLIBDIR)\libremote.lib" \
 "$(OUTLIBDIR)\libnet.lib" 

LIB_XAPIAN_OBJS= ".\xapian_wrap.obj" 


OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python
INTDIR=.\

# Debug builds of Python *insist* on a '_d' suffix for extension modules.
!if "$(DEBUG)" == "1"
PY_DEBUG_SUFFIX=_d
!else
PY_DEBUG_SUFFIX=
!endif


ALL : "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd" "$(OUTDIR)\xapian.py" "$(OUTDIR)\smoketest.py"

CLEAN :
	-@erase "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd"
	-@erase "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).exp"
	-@erase "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).lib"
	-@erase $(LIB_XAPIAN_OBJS)
	-@erase "$(OUTDIR)\xapian.py"
	-@erase "$(OUTDIR)\xapian.pyc"
	-@erase "$(OUTDIR)\xapian.pyo"
	-@erase "$(OUTDIR)\smoketest.py"
	-@erase "$(OUTDIR)\smoketest.pyc"
	-@erase "$(OUTDIR)\smoketest.pyo"
	
CLEANSWIG :	
	-@erase /Q /s modern
	-@erase generate-python-exceptions
	-@erase exception_data.pm 
		
DOTEST :
	cd "$(OUTDIR)"
	"$(PYTHON_EXE)" smoketest.py
	
CHECK: ALL DOTEST	

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_PYTHON)" /I "$(XAPIAN_CORE_REL_PYTHON)\include" \
 /I "$(PYTHON_INCLUDE)" /I "$(PYTHON_INCLUDE_2)" /I"." \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_DEPENDENCIES) "/LIBPATH:$(PYTHON_LIB_DIR)"

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: ../xapian.i util.i extra.i except.i 
	-erase /Q modern
	-md modern
	$(SWIG) -I"$(XAPIAN_CORE_REL_PYTHON)" -I"$(XAPIAN_CORE_REL_PYTHON)\include" -Werror -c++ -python -shadow -modern \
	    -outdir modern -o modern/xapian_wrap.cc ../xapian.i
	$(PERL_EXE) -pe 's/class Error:/class Error(Exception):/' modern/xapian.py > modern/xapian_py.tmp
	-erase modern/xapian.py
	-rename modern/xapian_py.tmp modern/xapian.py

"$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) \
                            $(XAPIAN_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<


except.i: generate-python-exceptions
	-copy "$(XAPIAN_CORE_REL_PYTHON)\exception_data.pm" exception_data.pm 
	$(PERL_EXE) generate-python-exceptions exception_data.pm 
		
generate-python-exceptions: generate-python-exceptions.in
	$(PERL_EXE) -pe 'BEGIN{$$perl=shift @ARGV} s,\@PERL\@,$$perl,' "$(PERL_EXE)" generate-python-exceptions.in > generate-python-exceptions

"$(OUTDIR)\xapian.py" : "modern\xapian.py"
	-copy $** "$(OUTDIR)\xapian.py"
	$(MANIFEST) "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd.manifest" -outputresource:"$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd;2"


"$(OUTDIR)\smoketest.py" : ".\smoketest.py"
	-copy $** "$(OUTDIR)\smoketest.py"

#
# Rules
#

".\xapian_wrap.obj" : "modern/xapian_wrap.cc"
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
