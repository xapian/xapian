## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = PYTHONPATH=".libs$(PYTHON_PATHSEP)." $(PYTHON)

## Test programs to be run
TESTS = smoketest.py

BUILT_SOURCES = modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py

EXTRA_DIST = util.i extra.i except.i $(TESTS) $(BUILT_SOURCES)

SUBDIRS = docs

pylibdir = @PYTHON_LIB@

# Install as _DATA rather than _SCRIPTS because we don't want to make these
# executable (they don't have a #! line).
pylib_DATA = xapian.py xapian.pyc

pylib_LTLIBRARIES = _xapian.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pylibdir)/_xapian.la

AM_CPPFLAGS = -I$(PYTHON_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
_xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
_xapian_la_SOURCES = modern/xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(PYTHON_LIBS)

# In a distribution, xapian.py is in $(srcdir)/modern, and python will always
# generate the .pyc file in the same directory as the .py file.  However,
# $(srcdir) might be shared between builds for different python versions
# so we can't let the .pyc file get written there.  Hence we copy the .py
# file to the build directory from wherever it actually is and use the copy.
xapian.py: modern/xapian.py
	cp `test -f modern/xapian.py || echo '$(srcdir)/'`modern/xapian.py .

# We "import _xapian" first so that if we fail to import the glue library
# we don't generate a broken xapian.pyc.
xapian.pyc: xapian.py _xapian.la
	PYTHONPATH=".$(PYTHON_PATHSEP).libs" $(PYTHON) -c "import _xapian;import xapian"

CLEANFILES = xapian.py xapian.pyc

if MAINTAINER_MODE
# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

BUILT_SOURCES += except.i
except.i: generate-python-exceptions ../../xapian-core/exception_data.pm
	./generate-python-exceptions

modern/xapian_wrap.cc modern/xapian_wrap.h modern/xapian.py: $(SWIG_sources) util.i except.i extra.i
	test -d modern || mkdir modern
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -python -shadow -modern -O -outdir modern \
	    -o modern/xapian_wrap.cc $(SWIG_mainsource)

CLEANFILES += $(BUILT_SOURCES)
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
