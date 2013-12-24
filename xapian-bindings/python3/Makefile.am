## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

## Test programs to be run
TESTS = smoketest.py pythontest.py
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)' ;\
	PYTHON='$(PYTHON3)' ;\
	export abs_builddir ;\
	export srcdir ;\
	export PYTHON ;
LOG_COMPILER = '$(srcdir)'/run-python-test

# FIXME: replicationtest.py attempts to test performing replication whilst
# modifications are in progress.  However, it sometimes fails due to a race
# condition in it.  It also contains hard-coded paths to the replication
# server and client, which assume the file layout in a repo checkout.
# Therefore, we don't run it by default currently.
# replicationtest.py

BUILT_SOURCES = modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py

EXTRA_DIST = python.i util.i extra.i extracomments.i except.i \
	generate-python-exceptions \
	module_top.py \
	run-python-test \
	testsuite.py \
	test_xapian_star.py \
	docs/index.rst \
	$(TESTS) $(BUILT_SOURCES)

pkgpylibdir = @PYTHON3_LIB@/xapian

# Install as _DATA rather than _SCRIPTS because we don't want to make these
# executable (they don't have a #! line).
pkgpylib_DATA = xapian/__init__.py \
		xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyc \
		xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyo

pkgpylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pkgpylibdir)/_xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(pkgpylibdir)/_xapian$(PYTHON3_SO)

AM_CPPFLAGS = -I$(PYTHON3_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module -shrext "$(PYTHON3_SO)" $(NO_UNDEFINED)
_xapian_la_SOURCES = modern/xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(PYTHON3_LIBS)

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
# we don't generate a broken .pyc or .pyo.
xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyc: xapian/__init__.py xapian/_xapian$(PYTHON3_SO)
	PYTHONPATH="xapian:$$PYTHONPATH" $(PYTHON3) -c "import _xapian"
	PYTHONPATH=".:$$PYTHONPATH" $(PYTHON3) -c "import xapian"

xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyo: xapian/__init__.py xapian/_xapian$(PYTHON3_SO)
	PYTHONPATH="xapian:$$PYTHONPATH" $(PYTHON3) -O -c "import _xapian"
	PYTHONPATH=".:$$PYTHONPATH" $(PYTHON3) -O -c "import xapian"

xapian/_xapian$(PYTHON3_SO): _xapian.la
	-test -d xapian || mkdir xapian
	$(LIBTOOL) --config > libtoolconfig.tmp
## ksh requires a path on the sourced file.
	. ./libtoolconfig.tmp; cp $$objdir/_xapian$(PYTHON3_SO) xapian
	rm -f libtoolconfig.tmp

CLEANFILES = \
    xapian/_xapian$(PYTHON3_SO) \
    xapian/__init__.py \
    xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyc \
    xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyo \
    $(srcdir)/*.pyc

# Clean the xapian directory which we created, if it's empty, and any
# databases created by test cases.
clean-local:
	-rmdir xapian
	rm -rf db_test_* dbs_replication

if MAINTAINER_MODE
BUILT_SOURCES += except.i doccomments.i
except.i: $(srcdir)/generate-python-exceptions ../../xapian-core/exception_data.pm
	$(PERL) -w -I$(srcdir)/../../xapian-core $(srcdir)/generate-python-exceptions

if MAINTAINER_NO_DOCS
doccomments.i:
	: > doccomments.i
else
doccomments.i: doxy2swig.py ../../xapian-core/exception_data.pm ../doxygen-xml/index.xml
	$(PYTHON) $(srcdir)/doxy2swig.py ../doxygen-xml/index.xml doccomments.i
endif

# Update the Python 3 versions from the Python 2 ones.
2to3:
	for a in pythontest replicationtest smoketest testsuite ; do cp ../python/$${a}.py $${a}.py ; 2to3 -w --no-diffs $${a}.py ; done

# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

stamp = modern/xapian_wrap.stamp
RUN_SWIG = stamp='$(stamp)' $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py modern/xapian_wrap.d: $(stamp)
	$(make_many_locked)
$(stamp): except.i doccomments.i
	-test -d modern || mkdir modern
	$(multitarget_begin)
	$(RUN_SWIG) $(SWIG_WERROR) -I. -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -python -threads -shadow -modern -O -outdir modern \
	    -o modern/xapian_wrap.cc '$(srcdir)/'python.i
	$(PERL) -pi -e 's/class Error:/class Error(Exception):/' modern/xapian.py
	$(multitarget_end)

-include modern/xapian_wrap.d

CLEANFILES += modern/xapian_wrap.d $(stamp)
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

if DOCUMENTATION_RULES
BUILT_SOURCES += docs/index.html

.rst.html:
	-case "$@" in */*) d=`echo "$@"|sed 's,/[^/]*$$,,'`; test -d "$$d" || mkdir "$$d" ;; esac
	$(RST2HTML) --exit-status=warning $< $@
endif
