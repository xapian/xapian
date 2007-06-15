# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libmulti.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libmulti.lib" 

LIBMULTI_OBJS= \
                $(INTDIR)\multi_postlist.obj \
                $(INTDIR)\multi_termlist.obj \
                $(INTDIR)\multi_alltermslist.obj
		
LOCAL_HEADERS =\
	$(INTDIR)\multi_postlist.h\
	$(INTDIR)\multi_termlist.h		

CLEAN :
	-@erase "$(OUTDIR)\libmulti.lib"
	-@erase "*.pch"
        -@erase $(LIBMULTI_OBJS)
	-@erase $(LIBMULTI_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBMULTI.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBMULTI_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libmulti.lib" $(DEF_FLAGS) $(LIBMULTI_OBJS)
<<

# if any headers change, rebuild all .objs
$(LIBMULTI_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

