if VPATH_BUILD
# We need this so that generated sources can find non-generated headers and
# non-generated sources can find generated headers in a VPATH build from git.
AM_CPPFLAGS += -I$(top_srcdir)/languages -Ilanguages
endif

noinst_HEADERS +=\
	languages/steminternal.h

snowball_algorithms =\
	languages/arabic.sbl\
	languages/armenian.sbl\
	languages/basque.sbl\
	languages/catalan.sbl\
	languages/danish.sbl\
	languages/dutch.sbl\
	languages/english.sbl\
	languages/earlyenglish.sbl\
	languages/finnish.sbl\
	languages/french.sbl\
	languages/german2.sbl\
	languages/german.sbl\
	languages/hungarian.sbl\
	languages/indonesian.sbl\
	languages/irish.sbl\
	languages/italian.sbl\
	languages/kraaij_pohlmann.sbl\
	languages/lithuanian.sbl\
	languages/lovins.sbl\
	languages/nepali.sbl\
	languages/norwegian.sbl\
	languages/porter.sbl\
	languages/portuguese.sbl\
	languages/romanian.sbl\
	languages/russian.sbl\
	languages/spanish.sbl\
	languages/swedish.sbl\
	languages/tamil.sbl\
	languages/turkish.sbl

snowball_built_sources =\
	$(snowball_algorithms:.sbl=.cc)\
	$(snowball_algorithms:.sbl=.h)

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

EXTRA_DIST += $(snowball_sources) $(snowball_headers) $(snowball_algorithms) $(snowball_built_sources)\
	languages/collate-sbl\
	languages/allsnowballheaders.h\
	languages/sbl-dispatch.h\
	languages/Makefile

stopworddir = $(pkgdatadir)/stopwords
dist_stopword_DATA = $(snowball_stopwords:.txt=.list)

snowball_stopwords = \
	languages/stopwords/arabic.txt\
	languages/stopwords/danish.txt\
	languages/stopwords/dutch.txt\
	languages/stopwords/english.txt\
	languages/stopwords/finnish.txt\
	languages/stopwords/french.txt\
	languages/stopwords/german.txt\
	languages/stopwords/hungarian.txt\
	languages/stopwords/indonesian.txt\
	languages/stopwords/italian.txt\
	languages/stopwords/norwegian.txt\
	languages/stopwords/portuguese.txt\
	languages/stopwords/russian.txt\
	languages/stopwords/spanish.txt\
	languages/stopwords/swedish.txt

.txt.list:
if VPATH_BUILD
	$(MKDIR_P) languages/stopwords
endif
	$(PERL) -pe 's/\|.*//g;s/\s+/\n/g;s/^\n//' $< |LC_COLLATE=C sort|uniq > $@

if MAINTAINER_MODE
$(snowball_built_sources): languages/snowball $(snowball_algorithms)

languages/snowball: $(snowball_sources) $(snowball_headers)
	$(CC_FOR_BUILD) -o languages/snowball \
	    -DDISABLE_CSHARP -DDISABLE_GO -DDISABLE_JAVA -DDISABLE_JS -DDISABLE_PASCAL -DDISABLE_PYTHON -DDISABLE_RUST \
	    `for f in $(snowball_sources) ; do test -f $$f && echo $$f || echo $(srcdir)/$$f ; done`

# /bin/tr on Solaris doesn't follow POSIX and requires [ and ] around ranges.
# With a POSIX-compliant tr, these are harmless as they mean replace [ with [
# and ] with ].
.sbl.cc:
	languages/snowball $< -o `echo $@|$(SED) 's!\.cc$$!!'` -c++ -u -n InternalStem`echo $<|$(SED) 's!.*/\(.\).*!\1!'|tr '[a-z]' '[A-Z]'``echo $<|$(SED) 's!.*/.!!;s!\.sbl!!'` -p SnowballStemImplementation

# /bin/tr on Solaris doesn't follow POSIX and requires [ and ] around ranges.
# With a POSIX-compliant tr, these are harmless as they mean replace [ with [
# and ] with ].
.sbl.h:
	languages/snowball $< -o `echo $@|$(SED) 's!\.h$$!!'` -c++ -u -n InternalStem`echo $<|$(SED) 's!.*/\(.\).*!\1!'|tr '[a-z]' '[A-Z]'``echo $<|$(SED) 's!.*/.!!;s!\.sbl!!'` -p SnowballStemImplementation

languages/allsnowballheaders.h: languages/sbl-dispatch.h
languages/sbl-dispatch.h languages/allsnowballheaders.h: languages/collate-sbl languages/Makefile.mk common/Tokeniseise.pm
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/languages/collate-sbl' '$(srcdir)' $(snowball_algorithms)

BUILT_SOURCES += $(snowball_built_sources)\
	languages/allsnowballheaders.h\
	languages/sbl-dispatch.h
CLEANFILES += languages/snowball
endif

lib_src += $(snowball_built_sources)\
	languages/stem.cc\
	languages/steminternal.cc
