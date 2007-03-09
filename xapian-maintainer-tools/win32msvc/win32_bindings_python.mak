# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

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
 "$(PYTHON_LIB)"

LIB_XAPIAN_OBJS= ".\xapian_wrap.obj" 


OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python
INTDIR=.\

ALL : "$(OUTDIR)\_xapian.pyd" "$(OUTDIR)\xapian.py" "$(OUTDIR)\smoketest.py"

CLEAN :
	-@erase "$(OUTDIR)\_xapian.pyd"
	-@erase "$(OUTDIR)\_xapian.exp"
	-@erase "$(OUTDIR)\_xapian.lib"
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

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_PYTHON)" /I "$(XAPIAN_CORE_REL_PYTHON)\include" \
 /I "$(PYTHON_INCLUDE)" /I"." \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_DEPENDENCIES)

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: ../xapian.i util.i extra.i except.i 
	-erase /Q modern
	-md modern
	$(SWIG) -I"$(XAPIAN_CORE_REL_PYTHON)" -I"$(XAPIAN_CORE_REL_PYTHON)\include" -Werror -c++ -python -shadow -modern \
	    -outdir modern -o modern/xapian_wrap.cc ../xapian.i

"$(OUTDIR)\_xapian.pyd" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) \
                            $(XAPIAN_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\_xapian.pyd" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<


except.i: generate-python-exceptions
	-copy "$(XAPIAN_CORE_REL_PYTHON)\exception_data.pm" exception_data.pm 
	$(PERL_EXE) generate-python-exceptions exception_data.pm 
		
generate-python-exceptions: generate-python-exceptions.in
	$(PERL_EXE) -pe 's,$(PERL_DIR),$(PERL_EXE),' generate-python-exceptions.in > generate-python-exceptions

"$(OUTDIR)\xapian.py" : "modern\xapian.py"
	-copy $** "$(OUTDIR)\xapian.py"
	$(MANIFEST) "$(OUTDIR)\_xapian.pyd.manifest" -outputresource:"$(OUTDIR)\_xapian.pyd;2"


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
