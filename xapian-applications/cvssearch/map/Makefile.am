bin_PROGRAMS = cvsmap
bin_SCRIPTS = cvsmap-script
EXTRA_DIST = cvsmap-script.pl

SUBDIRS = process range diff  aligned_diff alignment cvs_parser \
map_algorithm html_writer
INCLUDES = $(foreach dir, $(SUBDIRS), -I$(dir)) -I../db
cvsmap_SOURCES = map.cpp  collection.h  virtual_iostream.h  virtual_istream.h  virtual_ostream.h
cvsmap_LDADD = $(foreach dir, $(SUBDIRS), ./$(dir)/lib$(dir).a) ../db/libcvs_db.a \
 -lm -ldb_cxx $(foreach dir, $(SUBDIRS), ./$(dir)/lib$(dir).a) ../db/libcvs_db.a
CXXFLAGS = -O3 -DINLINE -DNDEBUG -Wall

define build-perl-script
$(PERL) -c $(firstword $^)
rm -rf $@
echo "#! " $(PERL) > $@
cat $(firstword $^) >> $@
chmod ugo+rx $@
endef

cvsmap-script : $(srcdir)/cvsmap-script.pl
	$(build-perl-script)
