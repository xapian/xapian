## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = trap 'rm -f "$(srcdir)"/*.pyc' 0 1 2 13 15; \
    PYTHONPATH=".:$$PYTHONPATH" PYTHONDONTWRITEBYTECODE=1 \
    abs_builddir=@abs_builddir@ $(PYTHON)

## Test programs to be run
TESTS = smoketest.py pythontest.py

# FIXME: replicationtest.py attempts to test performing replication whilst
# modifications are in progress.  However, it sometimes fails due to a race
# condition in it.  It also contains hard-coded paths to the replication
# server and client, which assume the file layout in a repo checkout.
# Therefore, we don't run it by default currently.
# replicationtest.py

BUILT_SOURCES = modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py

if !MAINTAINER_NO_DOCS
DOCCOMMENTS_I = doccomments.i
DOCCOMMENTS_I_FLAGS=-DDOCCOMMENTS_I_SOURCES
endif

EXTRA_DIST = util.i extra.i extracomments.i except.i \
	generate-python-exceptions \
	module_top.py \
	testsuite.py testsuite2.py testsuite3.py \
	test_xapian_star.py \
	smoketest2.py smoketest3.py \
	pythontest2.py pythontest3.py \
	replicationtest2.py \
	$(TESTS) $(BUILT_SOURCES)

pkgpylibdir = @PYTHON_LIB@/xapian

# Install as _DATA rather than _SCRIPTS because we don't want to make these
# executable (they don't have a #! line).
pkgpylib_DATA = xapian/__init__.py xapian/__init__.pyc xapian/__init__.pyo

pkgpylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pkgpylibdir)/_xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(pkgpylibdir)/_xapian$(PYTHON_SO)

AM_CPPFLAGS = -I$(PYTHON_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module -shrext "$(PYTHON_SO)" $(NO_UNDEFINED)
_xapian_la_SOURCES = modern/xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(PYTHON_LIBS)

# In a distribution, xapian.py is in $(srcdir)/modern, and python will always
# generate the .pyc and .pyo files in the same directory as the .py file.
# However, $(srcdir) might be shared between builds for different python
# versions so we can't let the .pyc and .pyo files get written there.  Hence we
# copy the .py file to the build directory from wherever it actually is and use
# the copy.  We also rename the copy to __init__.py so that it can be installed
# in a "xapian" directory and be treated as a package.  Finally, we take this
# opportunity to insert a module doc string at the top of the file.
xapian/__init__.py: modern/xapian.py
	-test -d xapian || mkdir xapian
	cat '$(srcdir)/module_top.py' `test -f modern/xapian.py || echo '$(srcdir)/'`modern/xapian.py > xapian/__init__.pyT
	mv xapian/__init__.pyT xapian/__init__.py

# We "import _xapian" first so that if we fail to import the glue library
# we don't generate a broken __init__.pyc or __init__.pyo.
xapian/__init__.pyc: xapian/__init__.py xapian/_xapian$(PYTHON_SO)
	PYTHONPATH="xapian:$$PYTHONPATH" $(PYTHON) -c "import _xapian"
	PYTHONPATH=".:$$PYTHONPATH" $(PYTHON) -c "import xapian"

xapian/__init__.pyo: xapian/__init__.py xapian/_xapian$(PYTHON_SO)
	PYTHONPATH="xapian:$$PYTHONPATH" $(PYTHON) -O -c "import _xapian"
	PYTHONPATH=".:$$PYTHONPATH" $(PYTHON) -O -c "import xapian"

xapian/_xapian$(PYTHON_SO): _xapian.la
	-test -d xapian || mkdir xapian
	$(LIBTOOL) --config > libtoolconfig.tmp
## ksh requires a path on the sourced file.
	. ./libtoolconfig.tmp; cp $$objdir/_xapian$(PYTHON_SO) xapian
	rm -f libtoolconfig.tmp

CLEANFILES = \
    xapian/_xapian$(PYTHON_SO) \
    xapian/__init__.py xapian/__init__.pyc xapian/__init__.pyo \
    $(srcdir)/*.pyc

# Clean the xapian directory which we created, if it's empty, and any
# databases created by test cases.
clean-local:
	-rmdir xapian
	rm -rf db_test_* dbs_replication

if MAINTAINER_MODE
# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

BUILT_SOURCES += except.i $(DOCCOMMENTS_I)
except.i: $(srcdir)/generate-python-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-python-exceptions

doccomments.i: doxy2swig.py ../../xapian-core/exception_data.pm ../../xapian-core/docs/apidoc/xml/index.xml
	$(PYTHON) $(srcdir)/doxy2swig.py ../../xapian-core/docs/apidoc/xml/index.xml doccomments.i

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py modern/xapian_wrap.d: modern/xapian_wrap.stamp
	@stamp=modern/xapian_wrap.stamp; $(make_many_locked)
modern/xapian_wrap.stamp: $(SWIG_sources) util.i except.i $(DOCCOMMENTS_I) extra.i extracomments.i
	-test -d modern || mkdir modern
	$(multitarget_begin)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) $(DOCCOMMENTS_I_FLAGS) -c++ \
	    -python -threads -shadow -modern -O -outdir modern \
	    -o modern/xapian_wrap.cc -MD -MF modern/xapian_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pe 's!modern/xapian_wrap.cc:!modern/xapian_wrap.stamp:!' modern/xapian_wrap_d.tmp > modern/xapian_wrap_d2.tmp
	mv modern/xapian_wrap_d2.tmp modern/xapian_wrap.d
	rm modern/xapian_wrap_d.tmp
	$(multitarget_end)

-include modern/xapian_wrap.d

CLEANFILES += modern/xapian_wrap.d modern/xapian_wrap.stamp
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/python/examples
dist_exampledata_DATA = \
	docs/examples/simpleindex.py \
	docs/examples/simpleexpand.py \
	docs/examples/simplematchdecider.py \
	docs/examples/simplesearch.py

docdatadir = $(docdir)/python
dist_docdata_DATA = docs/index.html
