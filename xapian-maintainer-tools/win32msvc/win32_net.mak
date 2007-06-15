# Makefile for Microsoft Visual C++ 7.0 (or compatible)

# Will build a Win32 static library libnet.lib

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\win32\config.mak


CPP=cl.exe
RSC=rc.exe


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libnet.lib" 

LIBNET_OBJS= \
             $(INTDIR)\progclient.obj \
             $(INTDIR)\remoteconnection.obj  \
             $(INTDIR)\remoteserver.obj  \
             $(INTDIR)\serialise.obj  \
             $(INTDIR)\tcpclient.obj  \
             $(INTDIR)\tcpserver.obj  \
             $(NULL)
	     
LOCAL_HEADERS= 


CLEAN :
	-@erase "$(OUTDIR)\libnet.lib"
	-@erase "*.pch" "*.pdb"
	-@erase $(LIBNET_OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)


"$(OUTDIR)\libnet.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBNET_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libnet.lib" $(DEF_FLAGS) $(LIBNET_OBJS)
<<

# if any headers change, rebuild all .objs
$(LIBNET_OBJS): $(LOCAL_HEADERS)

# inference rules, showing how to create one type of file from another with the same root name

{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
