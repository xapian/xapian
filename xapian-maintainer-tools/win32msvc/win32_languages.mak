# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd. www.lemurconsulting.com
# 17th March 2006
# Copyright (C) 2007, Olly Betts

# Will build a Win32 static library (non-debug) liblanguages.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

SBL_OPTIONS=-c++ -u 
SBL=compiler\snowball.exe

ALL : MAKEFROMSBL "$(OUTDIR)\liblanguages.lib" 

LIBLANGUAGES_OBJS= \
                 $(INTDIR)\stem.obj \
                 $(INTDIR)\steminternal.obj \
                 $(INTDIR)\danish.obj \
                 $(INTDIR)\dutch.obj \
                 $(INTDIR)\english.obj \
                 $(INTDIR)\finnish.obj \
                 $(INTDIR)\french.obj \
                 $(INTDIR)\german.obj \
		 $(INTDIR)\german2.obj \
		 $(INTDIR)\hungarian.obj \
                 $(INTDIR)\italian.obj \
                 $(INTDIR)\lovins.obj \
                 $(INTDIR)\norwegian.obj \
                 $(INTDIR)\porter.obj \
                 $(INTDIR)\portuguese.obj \
                 $(INTDIR)\russian.obj \
                 $(INTDIR)\spanish.obj \
                 $(INTDIR)\swedish.obj \
		 $(INTDIR)\romanian.obj \
		 $(INTDIR)\kraaij_pohlmann.obj \
		 $(INTDIR)\turkish.obj 

LIBLANGUAGES_SOURCES= \
                 $(INTDIR)\danish.cc \
                 $(INTDIR)\dutch.cc \
                 $(INTDIR)\english.cc \
                 $(INTDIR)\finnish.cc \
                 $(INTDIR)\french.cc \
                 $(INTDIR)\german.cc \
		 $(INTDIR)\german2.cc \
		 $(INTDIR)\hungarian.cc \
                 $(INTDIR)\italian.cc \
                 $(INTDIR)\lovins.cc \
                 $(INTDIR)\norwegian.cc \
                 $(INTDIR)\porter.cc \
                 $(INTDIR)\portuguese.cc \
                 $(INTDIR)\russian.cc \
                 $(INTDIR)\spanish.cc \
                 $(INTDIR)\swedish.cc \
		 $(INTDIR)\romanian.cc \
		 $(INTDIR)\kraaij_pohlmann.cc \
		 $(INTDIR)\turkish.cc 

LIBLANGUAGES_HEADERS= \
                 danish.h \
                 dutch.h \
                 english.h \
                 finnish.h \
                 french.h \
                 german.h \
		 german2.h \
		 hungarian.h \
                 italian.h \
                 lovins.h \
                 norwegian.h \
                 porter.h \
                 portuguese.h \
                 russian.h \
                 spanish.h \
                 swedish.h \
		 romanian.h \
		 kraaij_pohlmann.h \
		 turkish.h 

MAKEFROMSBL: $(LIBLANGUAGES_SOURCES) ".\allsnowballheaders.h"
		 
CLEAN :
	-@erase "$(OUTDIR)\liblanguages.lib"
	-@erase "*.pch"
	-@erase "$(INTDIR)\*.pdb"
        -@erase $(LIBLANGUAGES_OBJS)
	-@erase $(LIBLANGUAGES_SOURCES)
	-@erase $(LIBLANGUAGES_HEADERS)
	-@erase allsnowballheaders.h
	-@erase generate-allsnowballheaders

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBLANGUAGES.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBLANGUAGES_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\liblanguages.lib" $(DEF_FLAGS) $(LIBLANGUAGES_OBJS)
<<


# Generate .h and .cc files from Snowball algorithms using Snowball compiler
	
".\danish.h" ".\danish.cc" : ".\danish.sbl"
	$(SBL) danish.sbl $(SBL_OPTIONS) -o danish -n InternalStemDanish -p SnowballStemImplementation

".\dutch.h" ".\dutch.cc" : ".\dutch.sbl"
	$(SBL) dutch.sbl $(SBL_OPTIONS) -o dutch -n InternalStemDutch -p SnowballStemImplementation

".\english.h" ".\english.cc" : ".\english.sbl"
	$(SBL) english.sbl $(SBL_OPTIONS) -o english -n InternalStemEnglish -p SnowballStemImplementation

".\french.h" ".\french.cc" : ".\french.sbl"
	$(SBL) french.sbl $(SBL_OPTIONS) -o french -n InternalStemFrench -p SnowballStemImplementation

".\german.h" ".\german.cc" : ".\german.sbl"
	$(SBL) german.sbl $(SBL_OPTIONS) -o german -n InternalStemGerman -p SnowballStemImplementation

".\german2.h" ".\german2.cc" : ".\german2.sbl"
	$(SBL) german2.sbl $(SBL_OPTIONS) -o german2 -n InternalStemGerman2 -p SnowballStemImplementation

".\hungarian.h" ".\hungarian.cc" : ".\hungarian.sbl"
	$(SBL) hungarian.sbl $(SBL_OPTIONS) -o hungarian -n InternalStemHungarian -p SnowballStemImplementation

".\italian.h" ".\italian.cc" : ".\italian.sbl"
	$(SBL) italian.sbl $(SBL_OPTIONS) -o italian -n InternalStemItalian -p SnowballStemImplementation

".\norwegian.h" ".\norwegian.cc" : ".\norwegian.sbl"
	$(SBL) norwegian.sbl $(SBL_OPTIONS) -o norwegian -n InternalStemNorwegian -p SnowballStemImplementation

".\porter.h" ".\porter.cc" : ".\porter.sbl"
	$(SBL) porter.sbl $(SBL_OPTIONS) -o porter -n InternalStemPorter -p SnowballStemImplementation

".\portuguese.h" ".\portuguese.cc" : ".\portuguese.sbl"
	$(SBL) portuguese.sbl $(SBL_OPTIONS) -o portuguese -n InternalStemPortuguese -p SnowballStemImplementation

".\russian.h" ".\russian.cc" : ".\russian.sbl"
	$(SBL) russian.sbl $(SBL_OPTIONS) -o russian -n InternalStemRussian -p SnowballStemImplementation

".\spanish.h" ".\spanish.cc" : ".\spanish.sbl"
	$(SBL) spanish.sbl $(SBL_OPTIONS) -o spanish -n InternalStemSpanish -p SnowballStemImplementation

".\swedish.h" ".\swedish.cc" : ".\swedish.sbl"
	$(SBL) swedish.sbl $(SBL_OPTIONS) -o swedish -n InternalStemSwedish -p SnowballStemImplementation

".\kraaij_pohlmann.h" ".\kraaij_pohlmann.cc" : ".\kraaij_pohlmann.sbl"
	$(SBL) kraaij_pohlmann.sbl $(SBL_OPTIONS) -o kraaij_pohlmann -n InternalStemKraaij_pohlmann -p SnowballStemImplementation
		
".\romanian.h" ".\romanian.cc" : ".\romanian.sbl"
	$(SBL) romanian.sbl $(SBL_OPTIONS) -o romanian -n InternalStemRomanian -p SnowballStemImplementation	

".\turkish.h" ".\turkish.cc" : ".\turkish.sbl"
	$(SBL) turkish.sbl $(SBL_OPTIONS) -o turkish -n InternalStemTurkish -p SnowballStemImplementation	

".\finnish.h" ".\finnish.cc" : ".\finnish.sbl"
	$(SBL) finnish.sbl $(SBL_OPTIONS) -o finnish -n InternalStemFinnish -p SnowballStemImplementation

".\lovins.h" ".\lovins.cc" : ".\lovins.sbl"
	$(SBL) lovins.sbl $(SBL_OPTIONS) -o lovins -n InternalStemLovins -p SnowballStemImplementation


"$(INTDIR)\stem.obj" : ".\stem.cc" 
    $(CPP) @<<
  $(CPP_PROJ) $**
<<


"$(INTDIR)\steminternal.obj" : ".\steminternal.cc" $(LIBLANGUAGES_SOURCES) 
    $(CPP) @<<
  $(CPP_PROJ) $**
<<

".\allsnowballheaders.h": ".\generate-allsnowballheaders" 
    if not exist languages\. md languages
    $(PERL_EXE) generate-allsnowballheaders $(LIBLANGUAGES_HEADERS)
    copy languages\allsnowballheaders.h
    del languages\allsnowballheaders.h
    rmdir languages
 
".\generate-allsnowballheaders": ".\generate-allsnowballheaders.in" Makefile
    $(PERL_EXE) -pe "BEGIN{$$perl=shift @ARGV} s,\@PERL\@,$$perl," "$(PERL_EXE)" generate-allsnowballheaders.in > generate-allsnowballheaders

