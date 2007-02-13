SUFFIXES += .sbl

noinst_HEADERS +=\
	languages/steminternal.h

snowball_algorithms =\
	languages/danish.sbl\
	languages/dutch.sbl\
	languages/english.sbl\
	languages/finnish.sbl\
	languages/french.sbl\
	languages/german2.sbl\
	languages/german.sbl\
	languages/hungarian.sbl\
	languages/italian.sbl\
	languages/kraaij_pohlmann.sbl\
	languages/lovins.sbl\
	languages/norwegian.sbl\
	languages/porter.sbl\
	languages/portuguese.sbl\
	languages/romanian1.sbl\
	languages/romanian2.sbl\
	languages/russian.sbl\
	languages/spanish.sbl\
	languages/swedish.sbl

snowball_built_sources =\
	languages/danish.cc		languages/danish.h\
	languages/dutch.cc		languages/dutch.h\
	languages/english.cc		languages/english.h\
	languages/finnish.cc		languages/finnish.h\
	languages/french.cc		languages/french.h\
	languages/german2.cc		languages/german2.h\
	languages/german.cc		languages/german.h\
	languages/hungarian.cc		languages/hungarian.h\
	languages/italian.cc		languages/italian.h\
	languages/kraaij_pohlmann.cc	languages/kraaij_pohlmann.h\
	languages/lovins.cc		languages/lovins.h\
	languages/norwegian.cc		languages/norwegian.h\
	languages/porter.cc		languages/porter.h\
	languages/portuguese.cc		languages/portuguese.h\
	languages/romanian1.cc		languages/romanian1.h\
	languages/romanian2.cc		languages/romanian2.h\
	languages/russian.cc		languages/russian.h\
	languages/spanish.cc		languages/spanish.h\
	languages/swedish.cc		languages/swedish.h

snowball_sources =\
	languages/compiler/space.c\
	languages/compiler/tokeniser.c\
	languages/compiler/analyser.c\
	languages/compiler/generator.c\
	languages/compiler/driver.c
snowball_headers =\
	languages/compiler/header.h\
	languages/compiler/syswords.h\
	languages/compiler/syswords2.h

EXTRA_DIST += $(snowball_sources) $(snowball_headers) languages/allsnowballheaders.h\
	$(snowball_algorithms) $(snowball_built_sources)

if MAINTAINER_MODE
$(snowball_built_sources): languages/snowball $(snowball_algorithms)

languages/snowball: $(snowball_sources) $(snowball_headers)
	  $(CC_FOR_BUILD) -o languages/snowball -DDISABLE_JAVA `for f in $(snowball_sources) ; do test -f $$f && echo $$f || echo $(srcdir)/$$f ; done`

.sbl.cc:
	languages/snowball $< -o `echo $@|sed 's!\.cc$$!!'` -c++ -u -n InternalStem`echo $<|sed 's!.*/\(.\).*!\1!'|tr a-z A-Z``echo $<|sed 's!.*/.!!;s!\.sbl!!'` -p Stem::Internal

.sbl.h:
	languages/snowball $< -o `echo $@|sed 's!\.h$$!!'` -c++ -u -n InternalStem`echo $<|sed 's!.*/\(.\).*!\1!'|tr a-z A-Z``echo $<|sed 's!.*/.!!;s!\.sbl!!'` -p Stem::Internal

languages/allsnowballheaders.h: languages/Makefile.mk
	for f in $(snowball_built_sources) ; do case $$f in *.h) echo "#include \"$$f\"" ;; esac ; done > languages/allsnowballheaders.h.tmp
	echo '#define LANGSTRING "'`echo $(snowball_built_sources)|sed 's/[	 ][	 ]*/ /g;s!languages/[^ ]*\.cc languages/!!g;s!\.h!!g'`'"' >> languages/allsnowballheaders.h.tmp
	mv languages/allsnowballheaders.h.tmp languages/allsnowballheaders.h

BUILT_SOURCES += $(snowball_built_sources) languages/allsnowballheaders.h
CLEANFILES += languages/snowball $(snowball_built_sources) languages/allsnowballheaders.h
else
MAINTAINERCLEANFILES += languages/snowball $(snowball_built_sources) languages/allsnowballheaders.h
endif

libxapian_la_SOURCES += languages/stem.cc languages/steminternal.cc $(snowball_built_sources)
