## Process this file with automake to produce Makefile.in

xapianincludedir = $(includedir)/xapian

include_HEADERS = xapian.h

xapianinclude_HEADERS = xapian/base.h xapian/database.h xapian/document.h \
 xapian/enquire.h xapian/error.h xapian/errorhandler.h xapian/errortypes.h \
 xapian/expanddecider.h xapian/output.h xapian/positioniterator.h \
 xapian/postingiterator.h xapian/query.h xapian/stem.h xapian/termiterator.h \
 xapian/types.h xapian/valueiterator.h

nodist_xapianinclude_HEADERS = xapian/version.h

EXTRA_DIST = xapian/version.h.in
DISTCLEANFILES = xapian/version.h
