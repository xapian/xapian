## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = LUA_CPATH=".libs/?.so" $(LUA)

## Test programs to be run
TESTS = smoketest.lua

EXTRA_DIST = lua.i util.i except.i $(TESTS) $(BUILT_SOURCES)

lualibdir = @LUA_LIB@

lualib_LTLIBRARIES = xapian.la

# Remove the .la file - xapian.la is never linked against (it's a module)
# and LUA doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(lualibdir)/xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	eval `grep '^dlname=' $(lualib_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(lualibdir)/"$$dlname"

AM_CPPFLAGS = @LUA_CPPFLAGS@
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS) $(LUA_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-lua-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-lua-exceptions

xapian_wrap.cc:
	$(SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -lua \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp '$(srcdir)/'lua.i
	mv xapian_wrap_d.tmp xapian_wrap.d

-include xapian_wrap.d
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/lua/examples
dist_exampledata_DATA = \
	docs/examples/simpleindex.lua \
	docs/examples/simpleexpand.lua \
	docs/examples/simplematchdecider.lua \
	docs/examples/simplesearch.lua

docdatadir = $(docdir)/lua
dist_docdata_DATA = docs/index.html
