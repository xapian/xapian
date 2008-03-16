## Process this file with automake to produce Makefile.in

doc_sources = xapian-queryserver.but \
	      xapian-queryserver-manpage.but
#Disable documentation build until we can expect to have halibut on developers
#machines.
doc_targets =
doc_extra   =
doc_mans    =
#doc_targets = xapian-queryserver.txt \
#	      xapian-queryserver.html \
#	      xapian-queryserver.pdf \
#	      xapian-queryserver.info
#doc_extra   = xapian-queryserver.info-1
#doc_mans    = xapian-queryserver.1

EXTRA_DIST = $(doc_sources)

# Data to install, in the usual automake way
docdatadir   = $(datadir)/doc/xapian-queryserver
dist_docdata_DATA = $(doc_targets) $(doc_extra)
dist_man_MANS = $(doc_mans)

# Build documentation
all-local: $(doc_targets)

xapian-queryserver.txt: $(doc_sources)
	$(HALIBUT) --text=$@ $^

xapian-queryserver.html: $(doc_sources)
	$(HALIBUT) --html=$@ $^

xapian-queryserver.pdf: $(doc_sources)
	$(HALIBUT) --pdf=$@ $^

xapian-queryserver.info xapian-queryserver.info-1: $(doc_sources)
	$(HALIBUT) --info=xapian-queryserver.info $^

xapian-queryserver.1: xapian-queryserver-manpage.but
	$(HALIBUT) --man=xapian-queryserver.1 $^

# Clean up documentation
maintainer-clean-local:
	rm -f $(doc_targets) $(doc_extra) $(doc_mans)
