EXTRA_DIST +=\
	include/Makefile\
	include/xapian/Makefile

xapianincludedir = $(incdir)/xapian

inc_HEADERS =\
	include/xapian.h

xapianinclude_HEADERS =\
	include/xapian/attributes.h\
	include/xapian/cluster.h\
	include/xapian/compactor.h\
	include/xapian/constants.h\
	include/xapian/constinfo.h\
	include/xapian/database.h\
	include/xapian/dbfactory.h\
	include/xapian/deprecated.h\
	include/xapian/derefwrapper.h\
	include/xapian/diversify.h\
	include/xapian/document.h\
	include/xapian/enquire.h\
	include/xapian/eset.h\
	include/xapian/expanddecider.h\
	include/xapian/intrusive_ptr.h\
	include/xapian/iterator.h\
	include/xapian/keymaker.h\
	include/xapian/matchdecider.h\
	include/xapian/matchspy.h\
	include/xapian/mset.h\
	include/xapian/positioniterator.h\
	include/xapian/postingiterator.h\
	include/xapian/postingsource.h\
	include/xapian/query.h\
	include/xapian/queryparser.h\
	include/xapian/registry.h\
	include/xapian/rset.h\
	include/xapian/stem.h\
	include/xapian/termgenerator.h\
	include/xapian/termiterator.h\
	include/xapian/types.h\
	include/xapian/unicode.h\
	include/xapian/valueiterator.h\
	include/xapian/valuesetmatchdecider.h\
	include/xapian/geospatial.h\
	include/xapian/visibility.h\
	include/xapian/weight.h

nodist_xapianinclude_HEADERS =\
	include/xapian/version.h

# Regenerate include/xapian/version.h if its template has been changed.
all-local: include/xapian/version.h.timestamp

include/xapian/version.h.timestamp: include/xapian/version_h.cc
	$(SHELL) ./config.status --recheck

EXTRA_DIST +=\
	include/xapian/version_h.cc

DISTCLEANFILES +=\
	include/xapian/version.h\
	include/xapian/version.h.timestamp
