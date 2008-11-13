if VPATH_BUILD
# We need this so that generated sources can find non-generated headers in a
# VPATH build from SVN.
INCLUDES += -I$(top_srcdir)/geospatial

if MAINTAINER_MODE
# We need this because otherwise, if depcomp is being used (as it will be for a
# build with gcc-2.95), depcomp will be unable to find latlong_token.h.
# This may be a bug in depcomp, but it certainly happens with automake-1.10.
INCLUDES += -I$(top_builddir)/geospatial
endif
endif

noinst_HEADERS +=\
	geospatial/latlongparse_token.h

geospatial_lemon_built_sources =\
	geospatial/latlongparse_internal.cc\
	geospatial/latlongparse_token.h

EXTRA_DIST += $(geospatial_lemon_built_sources)\
	geospatial/dir_contents\
	geospatial/Makefile\
	geospatial/latlongparse.lemony

if MAINTAINER_MODE
geospatial/latlongparse_internal.cc geospatial/latlongparse_token.h: \
  geospatial/latlongparse_internal.stamp
## Recover from the removal of $@.  A full explanation of these rules is in the
## automake manual under the heading "Multiple Outputs".
	@if test -f $@; then :; else \
	  trap 'rm -rf geospatial/latlongparse_internal.lock geospatial/latlongparse_internal.stamp' 1 2 13 15; \
	  if mkdir geospatial/latlongparse_internal.lock 2>/dev/null; then \
	    rm -f geospatial/latlongparse_internal.stamp; \
	    $(MAKE) $(AM_MAKEFLAGS) geospatial/latlongparse_internal.stamp; \
	    rmdir geospatial/latlongparse_internal.lock; \
	  else \
	    while test -d geospatial/latlongparse_internal.lock; do sleep 1; done; \
	    test -f geospatial/latlongparse_internal.stamp; exit $$?; \
	  fi; \
	fi
# Lemon carefully avoids touching latlongparse_token.h if it hasn't changed,
# but only the generated file latlongparse_internal.cc depends
# on it, so it's better to touch it so we can have a
# dependency to generate it.
geospatial/latlongparse_internal.stamp: geospatial/latlongparse.lemony geospatial/latlongparse.lt queryparser/lemon
	queryparser/lemon -q -ogeospatial/latlongparse_internal.cc -hgeospatial/latlongparse_token.h $(srcdir)/geospatial/latlongparse.lemony && touch geospatial/latlongparse_token.h
	touch $@

BUILT_SOURCES += $(geospatial_lemon_built_sources)
CLEANFILES += $(geospatial_lemon_built_sources)
else
MAINTAINERCLEANFILES += $(geospatial_lemon_built_sources)
endif

libxapian_la_SOURCES +=\
	geospatial/geospatial.cc\
	geospatial/latlongparse_internal.cc
