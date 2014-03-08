## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

## Test programs to be run
TESTS = smoketest.lua
AM_TESTS_ENVIRONMENT = \
	LUA_CPATH='.libs/?$(LUA_SO)' ;\
	export LUA_CPATH ;
LOG_COMPILER = $(LUA)

EXTRA_DIST = lua.i util.i except.i \
	generate-lua-exceptions \
	docs/index.rst \
	$(TESTS) $(BUILT_SOURCES)

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
	rm -f $(DESTDIR)$(lualibdir)/xapian$(LUA_SO)

AM_CPPFLAGS = @LUA_CPPFLAGS@
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module -shrext "$(LUA_SO)" $(NO_UNDEFINED)
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
BUILT_SOURCES += except.i
except.i: $(srcdir)/generate-lua-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-lua-exceptions

RUN_SWIG = $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

xapian_wrap.cc: except.i
	$(RUN_SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -lua \
	    -o xapian_wrap.cc '$(srcdir)/'lua.i

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

if DOCUMENTATION_RULES
docdata_DATA = docs/index.html
BUILT_SOURCES += docs/index.html

.rst.html:
	-case "$@" in */*) d=`echo "$@"|sed 's,/[^/]*$$,,'`; test -d "$$d" || mkdir "$$d" ;; esac
	$(RST2HTML) --exit-status=warning $< $@
endif
