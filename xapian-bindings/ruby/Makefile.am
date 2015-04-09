## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = $(RUBY) -I.libs -I$(srcdir)

## Test programs to be run
TESTS = smoketest.rb

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h

EXTRA_DIST = util.i docs/rdocs $(TESTS) $(BUILT_SOURCES)

rubylibdir = $(RUBY_LIB)
dist_rubylib_DATA = xapian.rb

rubylibarchdir = $(RUBY_LIB_ARCH)
rubylibarch_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Ruby doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.la

AM_CPPFLAGS = -I$(RUBY_INC) -I$(RUBY_INC_ARCH)
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module -shrext ".$(RUBY_DLEXT)" $(NO_UNDEFINED)
_xapian_la_SOURCES = xapian_wrap.cc xapian_wrap.h
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(RUBY_LIBS)

CLEANFILES =
if MAINTAINER_MODE
xapian_wrap.cc xapian_wrap.h: xapian_wrap.stamp
	@stamp=xapian_wrap.stamp; $(make_many_locked)
xapian_wrap.stamp: $(SWIG_sources) util.i
	$(multitarget_begin)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -initname _xapian -ruby \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pi -e 's!xapian_wrap.cc:!xapian_wrap.stamp:!' xapian_wrap_d.tmp
	mv xapian_wrap_d.tmp xapian_wrap.d
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += xapian_wrap.stamp
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

# Compatibility wrapper which configure creates when required.
DISTCLEANFILES = rubyio.h

exampledatadir = $(docdir)/ruby/examples
dist_exampledata_DATA = \
	docs/examples/simpleexpand.rb \
	docs/examples/simpleindex.rb \
	docs/examples/simplematchdecider.rb \
	docs/examples/simplesearch.rb

docdatadir = $(docdir)/ruby
dist_docdata_DATA = docs/index.html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdatadir)
	@test -f docs/rdocs/index.html && cd docs || cd "$(srcdir)/docs" ; \
	  for dir in `find rdocs -type d -print` ; do \
	    echo " $(mkinstalldirs) '$(DESTDIR)$(docdatadir)/$$dir'"; \
	    $(mkinstalldirs) "$(DESTDIR)$(docdatadir)/$$dir"; \
	  done ; \
	  for f in `find rdocs -type f -print` ; do \
	    echo " $(INSTALL_DATA) '$$f' '$(DESTDIR)$(docdatadir)/$$f'"; \
	    $(INSTALL_DATA) "$$f" "$(DESTDIR)$(docdatadir)/$$f"; \
	  done

uninstall-local:
	## Because we don't install the .la file, "make uninstall" doesn't work
	## and we need to remove the file ourselves.
	rm -f $(DESTDIR)$(rubylibarchdir)/_xapian.$(RUBY_DLEXT)
	@test -f docs/rdocs/index.html && cd docs || cd "$(srcdir)/docs" ; \
	  for f in `find rdocs -type f -print` ; do \
	    echo " rm -f '$(DESTDIR)$(docdatadir)/$$f'"; \
	    rm -f "$(DESTDIR)$(docdatadir)/$$f"; \
	  done

EXTRA_DIST += generate-rdoc-stubs

if DOCUMENTATION_RULES
BUILT_SOURCES += docs/rdocs/index.html

docs/rdocs: docs/rdocs/index.html

docs/xapian.rb: generate-rdoc-stubs xapian.rb ../../xapian-core/docs/apidoc/xml/index.xml
	$(PERL) $(srcdir)/generate-rdoc-stubs

docs/rdocs/index.html: docs/xapian.rb
	rm -rf docs/rdocs
	cd docs && $(RDOC) --op rdocs xapian.rb
endif

CLEANFILES += docs/xapian.rb
