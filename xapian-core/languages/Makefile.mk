if VPATH_BUILD
# We need this so that generated sources can find non-generated headers and
# non-generated sources can find generated headers in a VPATH build from git.
AM_CPPFLAGS += -I$(top_srcdir)/languages -Ilanguages
endif

snowball_algorithms =\
	languages/algorithms/arabic.sbl\
	languages/algorithms/armenian.sbl\
	languages/algorithms/basque.sbl\
	languages/algorithms/catalan.sbl\
	languages/algorithms/danish.sbl\
	languages/algorithms/dutch_porter.sbl\
	languages/algorithms/dutch.sbl\
	languages/algorithms/earlyenglish.sbl\
	languages/algorithms/english.sbl\
	languages/algorithms/esperanto.sbl\
	languages/algorithms/estonian.sbl\
	languages/algorithms/finnish.sbl\
	languages/algorithms/french.sbl\
	languages/algorithms/german.sbl\
	languages/algorithms/greek.sbl\
	languages/algorithms/hindi.sbl\
	languages/algorithms/hungarian.sbl\
	languages/algorithms/indonesian.sbl\
	languages/algorithms/irish.sbl\
	languages/algorithms/italian.sbl\
	languages/algorithms/lithuanian.sbl\
	languages/algorithms/lovins.sbl\
	languages/algorithms/nepali.sbl\
	languages/algorithms/norwegian.sbl\
	languages/algorithms/polish.sbl\
	languages/algorithms/porter.sbl\
	languages/algorithms/portuguese.sbl\
	languages/algorithms/romanian.sbl\
	languages/algorithms/russian.sbl\
	languages/algorithms/serbian.sbl\
	languages/algorithms/spanish.sbl\
	languages/algorithms/swedish.sbl\
	languages/algorithms/tamil.sbl\
	languages/algorithms/turkish.sbl\
	languages/algorithms/yiddish.sbl

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
	languages/compiler/syswords.h

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
	$(CC_FOR_BUILD) -g -o languages/snowball -DTARGET_C_ONLY \
	    `for f in $(snowball_sources) ; do test -f $$f && echo $$f || echo $(srcdir)/$$f ; done`

.sbl.cc:
	languages/snowball $< -o $@ -c++ -u \
		-p 'Xapian::StemImplementation' \
		-P 'Xapian::Internal::Snowball' \
		-r 'languages/' \
		-cheader 'config.h' \
		-hheader 'xapian/stem.h'

.sbl.h:
	touch $<

languages/allsnowballheaders.h: languages/sbl-dispatch.h
languages/sbl-dispatch.h languages/allsnowballheaders.h: languages/collate-sbl languages/Makefile.mk common/Tokeniseise.pm
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/languages/collate-sbl' '$(srcdir)' $(snowball_algorithms)

BUILT_SOURCES += $(snowball_built_sources)\
	languages/allsnowballheaders.h\
	languages/sbl-dispatch.h
CLEANFILES += languages/snowball
endif

noinst_HEADERS +=\
	languages/api.h\
	languages/snowball_runtime.h

lib_src += $(snowball_built_sources)\
	languages/stem.cc \
	languages/utilities.cc
