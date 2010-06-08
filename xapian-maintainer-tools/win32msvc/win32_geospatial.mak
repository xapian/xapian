# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 25/Aug/09

# Will build a Win32 static library (non-debug) libgeospatial.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

OBJS= \
	$(INTDIR)\htmcalc.obj \
	$(INTDIR)\latlong_distance_keymaker.obj \
	$(INTDIR)\latlong_match_decider.obj \
	$(INTDIR)\latlong_posting_source.obj \
	$(INTDIR)\latlongcoord.obj \
	$(INTDIR)\latlongparse_internal.obj \
	$(INTDIR)\latlong_metrics.obj

SRCS= \
	$(INTDIR)\htmcalc.cc \
	$(INTDIR)\latlong_distance_keymaker.cc \
	$(INTDIR)\latlong_match_decider.cc \
	$(INTDIR)\latlong_posting_source.cc \
	$(INTDIR)\latlongcoord.cc \
	$(INTDIR)\latlongparse_internal.cc \
	$(INTDIR)\latlong_metrics.cc 

CPP_PROJ=$(CPPFLAGS_EXTRA) -I..\win32\ -Fo"$(INTDIR)\\" -Tp$(INPUTNAME) 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL : "$(OUTDIR)\libgeospatial.lib" 

CLEAN :
    -@erase "$(OUTDIR)\libgeospatial.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase $(OBJS)
    -@erase $(INTDIR)\latlongparse_token.h
    -@erase $(INTDIR)\latlongparse_internal.cc
    
"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(OUTDIR)\libgeospatial.lib" : $(INTDIR)\latlongparse_token.h "$(OUTDIR)" $(DEF_FILE) $(OBJS) 
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libgeospatial.lib" $(DEF_FLAGS) $(OBJS)
<<

$(INTDIR)\latlongparse_internal.cc $(INTDIR)\latlongparse_token.h: $(INTDIR)\latlongparse.lemony $(INTDIR)\latlongparse.lt 
	..\queryparser\lemon -q -o"$(INTDIR)\latlongparse_internal.cc" -h"$(INTDIR)\latlongparse_token.h" $(INTDIR)\latlongparse.lemony 


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
    if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) 
# DO NOT DELETE THIS LINE -- xapdep depends on it.
