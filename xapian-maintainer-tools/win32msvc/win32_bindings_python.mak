# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build the Python bindings 

# Where the core is, relative to the Python bindings
# Change this to match your environment
XAPIAN_CORE_REL_PYTHON=..\..\xapian-core

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

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



CPP=cl.exe
RSC=rc.exe

OUTDIR=$(XAPIAN_CORE_REL_PYTHON)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Python
INTDIR=.\

ALL : "$(OUTDIR)\_xapian.dll" "$(OUTDIR)\xapian.py" "$(OUTDIR)\smoketest.py"

CLEAN :
	-@erase "$(OUTDIR)\_xapian.dll"
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
	-@erase /Q /s olde
	
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

LIB32=link.exe 
LIB32_FLAGS=/nologo  $(LIBFLAGS) \
 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console \
 $(XAPIAN_DEPENDENCIES)



modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: ../xapian.i util.i extra.i
	-erase /Q modern
	-md modern
	$(SWIG) -I"$(XAPIAN_CORE_REL_PYTHON)" -I"$(XAPIAN_CORE_REL_PYTHON)\include" -Werror -c++ -python -shadow -modern \
	    -outdir modern -o modern/xapian_wrap.cc ../xapian.i

olde/xapian_wrap.cc olde/xapian_wrap.h olde/xapian.py: ../xapian.i util.i extra.i
	-erase /Q olde
	-md olde
	$(SWIG) -I"$(XAPIAN_CORE_REL_PYTHON)" -I"$(XAPIAN_CORE_REL_PYTHON)\include"  -Werror -c++ -python -shadow \
	    -outdir olde -o olde/xapian_wrap.cc ../xapian.i






"$(OUTDIR)\_xapian.dll" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) \
                            $(XAPIAN_DEPENDENCIES)
    $(LIB32) @<<
  $(LIB32_FLAGS) /DLL /out:"$(OUTDIR)\_xapian.dll" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<


"$(OUTDIR)\xapian.py" : "$(PYTHON_MODERN_OR_OLDE)\xapian.py"
	-copy $** "$(OUTDIR)\xapian.py"

"$(OUTDIR)\smoketest.py" : ".\smoketest.py"
	-copy $** "$(OUTDIR)\smoketest.py"

#
# Rules
#

".\xapian_wrap.obj" : "$(PYTHON_MODERN_OR_OLDE)\xapian_wrap.cc"
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
