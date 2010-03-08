# Makefile for Microsoft Visual C++ 7.0 (or compatible)

# Will build a Win32 static library libnet.lib

!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libnet.lib" 

OBJS= \
             $(INTDIR)\progclient.obj \
             $(INTDIR)\remoteconnection.obj  \
             $(INTDIR)\remoteserver.obj  \
             $(INTDIR)\serialise.obj  \
             $(INTDIR)\tcpclient.obj  \
             $(INTDIR)\tcpserver.obj  \
             $(INTDIR)\replicatetcpclient.obj \
             $(INTDIR)\remotetcpclient.obj \
             $(INTDIR)\replicatetcpserver.obj \
             $(INTDIR)\remotetcpserver.obj
             
SRCS= \
             $(INTDIR)\progclient.cc \
             $(INTDIR)\remoteconnection.cc  \
             $(INTDIR)\remoteserver.cc  \
             $(INTDIR)\serialise.cc  \
             $(INTDIR)\tcpclient.cc  \
             $(INTDIR)\tcpserver.cc \
             $(INTDIR)\replicatetcpclient.cc \
             $(INTDIR)\remotetcpclient.cc \
             $(INTDIR)\replicatetcpserver.cc \
             $(INTDIR)\remotetcpserver.cc

CLEAN :
	-@erase "$(OUTDIR)\libnet.lib"
	-@erase "*.pch" "*.pdb"
	-@erase "$(INTDIR)\*.pdb"
	-@erase $(OBJS)

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=/nologo  $(LIBFLAGS)


"$(OUTDIR)\libnet.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libnet.lib" $(DEF_FLAGS) $(OBJS)
<<

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