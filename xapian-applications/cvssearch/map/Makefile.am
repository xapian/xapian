bin_PROGRAMS = cvssearch
bin_SCRIPTS = cvssearch-script
EXTRA_DIST = cvssearch-script.pl

SUBDIRS = process range diff  aligned_diff alignment cvs_parser map_algorithm
INCLUDES = $(foreach dir, $(SUBDIRS), -I$(dir))
cvssearch_SOURCES = cvssearch.cpp  collection.h  virtual_iostream.h  virtual_istream.h  virtual_ostream.h
cvssearch_LDADD = $(foreach dir, $(SUBDIRS), ./$(dir)/lib$(dir).a) \
              -lm $(foreach dir, $(SUBDIRS), ./$(dir)/lib$(dir).a)
CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall
cvssearch-script : $(srcdir)/cvssearch-script.pl
	$(PERL) -c cvssearch-script.pl
	rm -rf cvssearch-script
	echo "#! " $(PERL) > cvssearch-script
	cat $(srcdir)/cvssearch-script.pl >> cvssearch-script
	chmod ugo+x cvssearch-script
