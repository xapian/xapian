## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = trap 'rm -f "$(srcdir)"/testsuite.pyc' 0 1 2 13 15; \
    PYTHONPATH=".:$$PYTHONPATH" PYTHONDONTWRITEBYTECODE=1 $(PYTHON)

## Test programs to be run
TESTS = smoketest.py pythontest.py

BUILT_SOURCES = modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py

if !MAINTAINER_NO_DOCS
DOCCOMMENTS_I = doccomments.i
DOCCOMMENTS_I_FLAGS=-DDOCCOMMENTS_I_SOURCES
endif

EXTRA_DIST = util.i extra.i extracomments.i except.i $(DOCCOMMENTS_I) \
	testsuite.py $(TESTS) $(BUILT_SOURCES)

SUBDIRS = docs

pylibdir = @PYTHON_LIB@

# Install as _DATA rather than _SCRIPTS because we don't want to make these
# executable (they don't have a #! line).
pylib_DATA = xapian.py xapian.pyc xapian.pyo

pylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pylibdir)/_xapian.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(pylibdir)/_xapian$(PYTHON_SO)

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
# the copy.
xapian.py: modern/xapian.py
	cp `test -f modern/xapian.py || echo '$(srcdir)/'`modern/xapian.py .

# We "import _xapian" first so that if we fail to import the glue library
# we don't generate a broken xapian.pyc or xapian.pyo.
xapian.pyc: xapian.py _xapian$(PYTHON_SO)
	PYTHONPATH=. $(PYTHON) -c "import _xapian;import xapian"

xapian.pyo: xapian.py _xapian$(PYTHON_SO)
	PYTHONPATH=. $(PYTHON) -O -c "import _xapian;import xapian"

_xapian$(PYTHON_SO): _xapian.la
	$(LIBTOOL) --config > libtoolconfig.tmp
	. ./libtoolconfig.tmp; cp $$objdir/_xapian$(PYTHON_SO) .
	rm -f libtoolconfig.tmp

CLEANFILES = \
    _xapian$(PYTHON_SO) \
    xapian.py xapian.pyc xapian.pyo \
    $(srcdir)/testsuite.pyc

# Clean any databases created by test cases.
clean-local:
	rm -rf db_test_*

if MAINTAINER_MODE
# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

BUILT_SOURCES += except.i $(DOCCOMMENTS_I)
except.i: generate-python-exceptions ../../xapian-core/exception_data.pm
	./generate-python-exceptions

doccomments.i: doxy2swig.py ../../xapian-core/exception_data.pm ../../xapian-core/docs/apidoc/xml/index.xml
	$(PYTHON) $(srcdir)/doxy2swig.py ../../xapian-core/docs/apidoc/xml/index.xml doccomments.i

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: modern/xapian_wrap.stamp
	@stamp=modern/xapian_wrap.stamp; $(make_many_locked)
modern/xapian_wrap.stamp: $(SWIG_sources) util.i except.i $(DOCCOMMENTS_I) extra.i extracomments.i
	-test -d modern || mkdir modern
	$(multitarget_begin)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) $(DOCCOMMENTS_I_FLAGS) -c++ \
	    -python -threads -shadow -modern -O -outdir modern \
	    -o modern/xapian_wrap.cc $(SWIG_mainsource)
	$(PERL) -pe 's/class Error:/class Error(Exception):/' modern/xapian.py > modern/xapian_py.tmp
	mv modern/xapian_py.tmp modern/xapian.py
	$(multitarget_end)

CLEANFILES += $(BUILT_SOURCES) modern/xapian_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
