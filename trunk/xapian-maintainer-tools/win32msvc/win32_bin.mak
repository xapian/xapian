# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build the binary tools


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
OUTLIBDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

PROGRAMS = \
           "$(OUTDIR)\xapian-compact.exe" \
           "$(OUTDIR)\xapian-progsrv.exe" \
           "$(OUTDIR)\xapian-chert-update.exe" \
           "$(OUTDIR)\xapian-tcpsrv.exe" \
           "$(OUTDIR)\xapian-inspect.exe" \
           "$(OUTDIR)\xapian-check.exe" \
           "$(OUTDIR)\xapian-replicate.exe" \
           "$(OUTDIR)\xapian-replicate-server.exe"\
           
SRCS = \
	"$(INTDIR)\xapian-compact.cc" \
	"$(INTDIR)\xapian-chert-update.cc" \
	"$(INTDIR)\xapian-progsrv.cc" \
	"$(INTDIR)\xapian-tcpsrv.cc" \
	"$(INTDIR)\xapian-inspect.cc" \
	"$(INTDIR)\xapian-check-chert.cc" \
	"$(INTDIR)\xapian-check-brass.cc" \
	"$(INTDIR)\xapian-check-flint.cc" \
	"$(INTDIR)\xapian-check.cc" \
    "$(INTDIR)\xapian-replicate.cc" \
    "$(INTDIR)\xapian-replicate-server.cc"

	   
ALL : $(PROGRAMS)

XAPIAN_COMPACT_OBJS= "$(INTDIR)\xapian-compact.obj" 

XAPIAN_CHERT_UPDATE_OBJS= "$(INTDIR)\xapian-chert-update.obj" 

XAPIAN_PROGSRV_OBJS= "$(INTDIR)\xapian-progsrv.obj" 

XAPIAN_TCPSRV_OBJS= "$(INTDIR)\xapian-tcpsrv.obj" 

XAPIAN_INSPECT_OBJS= "$(INTDIR)\xapian-inspect.obj" 

XAPIAN_CHECK_OBJS= \
	"$(INTDIR)\xapian-check.obj" \
	"$(INTDIR)\xapian-check-flint.obj" \
	"$(INTDIR)\xapian-check-chert.obj" \
	"$(INTDIR)\xapian-check-brass.obj" 

XAPIAN_REPLICATE_OBJS= "$(INTDIR)\xapian-replicate.obj" 

XAPIAN_REPLICATE_SERVER_OBJS= "$(INTDIR)\xapian-replicate-server.obj" 

	
CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(XAPIAN_COMPACT_OBJS)
    -@erase $(XAPIAN_CHERT_UPDATE_OBJS)
	-@erase $(XAPIAN_PROGSRV_OBJS)
	-@erase $(XAPIAN_TCPSRV_OBJS)
	-@erase $(XAPIAN_INSPECT_OBJS)
	-@erase $(XAPIAN_CHECK_OBJS)    
	-@erase $(XAPIAN_REPLICATE_OBJS)
	-@erase $(XAPIAN_REPLICATE_SERVER_OBJS)
	-@erase "$(INTDIR)\*.pdb"


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I ".." /I "..\testsuite"  /I"..\backends\flint" /I"..\backends\chert" /I"..\backends\brass"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tp$(INPUTNAME)

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS)
 
PROGRAM_DEPENDENCIES = $(XAPIAN_LIBS)


"$(OUTDIR)\xapian-compact.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_COMPACT_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-compact.exe" $(DEF_FLAGS) $(XAPIAN_COMPACT_OBJS)
<<
# REMOVE THIS NEXT LINE if using Visual C++ .net 2003 - you won't need to worry about manifests. For later compilers this prevents error R6034
    $(MANIFEST) "$(OUTDIR)\xapian-compact.exe.manifest" -outputresource:"$(OUTDIR)\xapian-compact.exe;1"
    -@erase "$(OUTDIR)\xapian-compact.exe.manifest"

"$(OUTDIR)\xapian-progsrv.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_PROGSRV_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-progsrv.exe" $(DEF_FLAGS) $(XAPIAN_PROGSRV_OBJS)
<<
    $(MANIFEST) "$(OUTDIR)\xapian-progsrv.exe.manifest" -outputresource:"$(OUTDIR)\xapian-progsrv.exe;1"
    -@erase "$(OUTDIR)\xapian-progsrv.exe.manifest"

"$(OUTDIR)\xapian-chert-update.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_CHERT_UPDATE_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-chert-update.exe" $(DEF_FLAGS) $(XAPIAN_CHERT_UPDATE_OBJS)
<<
    $(MANIFEST) "$(OUTDIR)\xapian-chert-update.exe.manifest" -outputresource:"$(OUTDIR)\xapian-chert-update.exe;1"
    -@erase "$(OUTDIR)\xapian-chert-update.exe.manifest"
    
"$(OUTDIR)\xapian-tcpsrv.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_TCPSRV_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-tcpsrv.exe" $(DEF_FLAGS) $(XAPIAN_TCPSRV_OBJS)
<<
    $(MANIFEST) "$(OUTDIR)\xapian-tcpsrv.exe.manifest" -outputresource:"$(OUTDIR)\xapian-tcpsrv.exe;1"
    -@erase "$(OUTDIR)\xapian-tcpsrv.exe.manifest"

"$(OUTDIR)\xapian-inspect.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_INSPECT_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-inspect.exe" $(DEF_FLAGS) $(XAPIAN_INSPECT_OBJS)
<<
    $(MANIFEST) "$(OUTDIR)\xapian-inspect.exe.manifest" -outputresource:"$(OUTDIR)\xapian-inspect.exe;1"
    -@erase "$(OUTDIR)\xapian-inspect.exe.manifest"


"$(OUTDIR)\xapian-check.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_CHECK_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-check.exe" $(DEF_FLAGS) $(XAPIAN_CHECK_OBJS) "$(OUTLIBDIR)\libflintbtreecheck.lib" "$(OUTLIBDIR)\libchertbtreecheck.lib" "$(OUTLIBDIR)\libbrassbtreecheck.lib"
<<
    $(MANIFEST) "$(OUTDIR)\xapian-check.exe.manifest" -outputresource:"$(OUTDIR)\xapian-check.exe;1"
    -@erase "$(OUTDIR)\xapian-check.exe.manifest"


"$(OUTDIR)\xapian-replicate.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_REPLICATE_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-replicate.exe" $(DEF_FLAGS) $(XAPIAN_REPLICATE_OBJS)
<<
    $(MANIFEST) "$(OUTDIR)\xapian-replicate.exe.manifest" -outputresource:"$(OUTDIR)\xapian-replicate.exe;1"
    -@erase "$(OUTDIR)\xapian-replicate.exe.manifest"

"$(OUTDIR)\xapian-replicate-server.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_REPLICATE_SERVER_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-replicate-server.exe" $(DEF_FLAGS) $(XAPIAN_REPLICATE_SERVER_OBJS)
<<
    $(MANIFEST) "$(OUTDIR)\xapian-replicate-server.exe.manifest" -outputresource:"$(OUTDIR)\xapian-replicate-server.exe;1"
    -@erase "$(OUTDIR)\xapian-replicate.exe.manifest"

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

