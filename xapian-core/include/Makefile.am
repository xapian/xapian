## Process this file with automake to produce Makefile.in

xapianincludedir = $(includedir)/xapian

include_HEADERS = xapian.h

xapianinclude_HEADERS = xapian/base.h xapian/database.h xapian/dbfactory.h \
 xapian/deprecated.h \
 xapian/document.h xapian/enquire.h xapian/error.h xapian/errorhandler.h \
 xapian/errortypes.h xapian/expanddecider.h xapian/output.h \
 xapian/positioniterator.h xapian/postingiterator.h xapian/query.h \
 xapian/queryparser.h xapian/stem.h xapian/termiterator.h xapian/types.h \
 xapian/valueiterator.h

nodist_xapianinclude_HEADERS = xapian/version.h

# Regenerate xapian/version.h if its template has been changed.
all-local: xapian/version.h.timestamp

xapian/version.h.timestamp: xapian/version_h.cc
	cd $(top_builddir) && $(SHELL) ./config.status --recheck

EXTRA_DIST = xapian/version_h.cc
DISTCLEANFILES = xapian/version.h xapian/version.h.timestamp
