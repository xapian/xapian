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

LIB_XAPIAN_OBJS= ".\xapian_wrap.obj" ".\version.res" 

INTDIR=.\

# Debug builds of Python *insist* on a '_d' suffix for extension modules.
!if "$(DEBUG)" == "1"
PY_DEBUG_SUFFIX=_d
!else
PY_DEBUG_SUFFIX=
!endif

!if "$(PYTHON_VER)" == "24"
PYTHON_EXE = $(PYTHON_EXE_24)
PYTHON_INCLUDE = $(PYTHON_INCLUDE_24)
PYTHON_INCLUDE_2 = $(PYTHON_INCLUDE_2_24)
PYTHON_LIB_DIR = $(PYTHON_LIB_DIR_24)
OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python24
!else if "$(PYTHON_VER)" == "25"
PYTHON_EXE = $(PYTHON_EXE_25)
PYTHON_INCLUDE = $(PYTHON_INCLUDE_25)
PYTHON_INCLUDE_2 = $(PYTHON_INCLUDE_2_25)
PYTHON_LIB_DIR= $(PYTHON_LIB_DIR_25)
OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python25
!else if "$(PYTHON_VER)" == "26"
PYTHON_EXE = $(PYTHON_EXE_26)
PYTHON_INCLUDE = $(PYTHON_INCLUDE_26)
PYTHON_INCLUDE_2 = $(PYTHON_INCLUDE_2_26)
PYTHON_LIB_DIR= $(PYTHON_LIB_DIR_26)
OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python26
!else if "$(PYTHON_VER)" == "27"
PYTHON_EXE = $(PYTHON_EXE_27)
PYTHON_INCLUDE = $(PYTHON_INCLUDE_27)
PYTHON_INCLUDE_2 = $(PYTHON_INCLUDE_2_27)
PYTHON_LIB_DIR= $(PYTHON_LIB_DIR_27)
OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python27
!else 
# Must specify a version
exit(1)
!endif

PYTHON_PACKAGE=xapian

ALL : "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd" "$(OUTDIR)\xapian.py" 

CLEANLOCAL:
    -@erase *.pdb
    -@erase *.res
    -@erase *.obj
    -@erase pythonversion.h

CLEAN : CLEANLOCAL
	-@erase $(LIB_XAPIAN_OBJS)
	-@erase /Q /s "$(OUTDIR)\$(PYTHON_PACKAGE)"
	-@erase /Q /s "$(OUTDIR)\build"
	-@erase /Q /s "$(OUTDIR)\dist"
	-@erase /Q /s "$(OUTDIR)"
	-@erase "$(OUTDIR)\*.pdb"
	
CLEANSWIG : CLEAN
	-@erase /Q /s modern
	-@erase exception_data.pm 
	-@erase except.i 
		
DOTEST :
    copy pythontest*.py "$(OUTDIR)"
    copy testsuite*.py "$(OUTDIR)"
    copy smoketest*.py "$(OUTDIR)"
    copy replication*.py "$(OUTDIR)"
    copy test_xapian_star.py "$(OUTDIR)"
	cd "$(OUTDIR)"
	copy "$(ZLIB_BIN_DIR)\zlib1.dll"
	"$(PYTHON_EXE)" smoketest.py
	"$(PYTHON_EXE)" pythontest.py    
# FIXME "$(PYTHON_EXE)" replicationtest.py
	
CHECK: ALL DOTEST	

DIST: "$(OUTDIR)\$(PYTHON_PACKAGE)" CHECK
    cd "$(MAKEDIR)"
    copy setup.py "$(OUTDIR)"
    if not exist "$(OUTDIR)\$(PYTHON_PACKAGE)\docs" mkdir "$(OUTDIR)\$(PYTHON_PACKAGE)\docs"
    if not exist "$(OUTDIR)\$(PYTHON_PACKAGE)\docs\examples" mkdir "$(OUTDIR)\$(PYTHON_PACKAGE)\docs\examples"
    copy docs\*.html $(OUTDIR)\$(PYTHON_PACKAGE)\docs
    copy docs\examples\*.py $(OUTDIR)\$(PYTHON_PACKAGE)\docs\examples
    cd "$(OUTDIR)"
    copy zlib1.dll $(PYTHON_PACKAGE)
    copy "_xapian$(PY_DEBUG_SUFFIX).pyd" $(PYTHON_PACKAGE)
    copy xapian.py "$(PYTHON_PACKAGE)\__init__.py"
    "$(PYTHON_EXE)" setup.py bdist_wininst
    

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"
    
"$(OUTDIR)\$(PYTHON_PACKAGE)" : "$(OUTDIR)"
    if not exist "$(OUTDIR)/$(PYTHON_PACKAGE)/$(NULL)" mkdir "$(OUTDIR)\$(PYTHON_PACKAGE)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I"$(XAPIAN_CORE_REL_PYTHON)" /I"$(XAPIAN_CORE_REL_PYTHON)\include" \
 /I"$(PYTHON_INCLUDE)" /I"$(PYTHON_INCLUDE_2)" /I"." \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) "/LIBPATH:$(PYTHON_LIB_DIR)" 

!IF "$(SWIGBUILD)" == "1"
modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: ../xapian.i util.i except.i doccomments.i extra.i extracomments.i
    -rd /s/q modern
	-md modern
	$(SWIG) $(SWIG_FLAGS) -I$(XAPIAN_CORE_REL_PYTHON)\include \
        -DDOCCOMMENTS_I_SOURCES -c++ -python -threads -shadow -modern -O -outdir modern \
	    -o modern/xapian_wrap.cc ../xapian.i       
        
	$(PERL_EXE) -pe "s/class Error:/class Error(Exception):/" modern\xapian.py > modern\xapian_py.tmp
	-erase modern\xapian.py
	-rename modern\xapian_py.tmp xapian.py
!ENDIF

"$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) 
                            
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<

  
except.i: generate-python-exceptions
	-copy "$(XAPIAN_CORE_REL_PYTHON)\exception_data.pm" exception_data.pm 
	$(PERL_EXE) generate-python-exceptions exception_data.pm 
		
"$(OUTDIR)\xapian.py" : "modern\xapian.py"
	-copy $** "$(OUTDIR)\xapian.py"
	$(MANIFEST) "$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd.manifest" -outputresource:"$(OUTDIR)\_xapian$(PY_DEBUG_SUFFIX).pyd;2"


#
# Rules
#

".\version.res": version.rc
    "$(PYTHON_EXE)" -c \
	"import platform; \
	f=open('pythonversion.h','w'); \
	f.write('#define PYTHON_VERSION \"'); \
	f.write(platform.python_version()); \
	f.write('\"\n'); \
	f.close();"
    $(RSC) /v \
      /fo version.res \
      /I"$(XAPIAN_CORE_REL_PYTHON)\include" \
      version.rc 


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
