bin_PROGRAMS = cvsmap
bin_SCRIPTS = map-script
EXTRA_DIST = map-script.pl

SUBDIRS = process range diff  aligned_diff alignment cvs_parser map_algorithm
INCLUDES = $(foreach dir, $(SUBDIRS), -I$(dir))
cvsmap_SOURCES = map.cpp  collection.h  virtual_iostream.h  virtual_istream.h  virtual_ostream.h
cvsmap_LDADD = $(foreach dir, $(SUBDIRS), ./$(dir)/lib$(dir).a) \
              -lm $(foreach dir, $(SUBDIRS), ./$(dir)/lib$(dir).a)
CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall
map-script : $(srcdir)/map-script.pl
	$(PERL) -c map-script.pl
	rm -rf map-script
	echo "#! " $(PERL) > map-script
	cat $(srcdir)/map-script.pl >> map-script
	chmod ugo+x map-script
