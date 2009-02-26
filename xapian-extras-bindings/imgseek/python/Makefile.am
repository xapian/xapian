## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

include ../../generic/generic.mk

SWIG_mainsource = \
	$(srcdir)/../imgseek.i

SWIG_sources = \
	$(SWIG_mainsource) \
	$(srcdir)/../../../xapian-bindings/generic/except.i

SWIG_includes = \
	-I$(srcdir) \
	-I$(srcdir)/../../../xapian-extras/imgseek/include \
	-I$(srcdir)/../../../xapian-bindings/python \
	-I$(srcdir)/../../../xapian-bindings/generic \
	-I$(srcdir)/../../../xapian-bindings

TESTS_ENVIRONMENT = PYTHONPATH=".:$$PYTHONPATH" abs_builddir=@abs_builddir@ $(PYTHON)

## Test programs to be run
TESTS = pythontest.py

BUILT_SOURCES = imgseek_wrap.cc imgseek_wrap.h imgseek.py

# if !MAINTAINER_NO_DOCS
# DOCCOMMENTS_I = imgseek_doccomments.i
# DOCCOMMENTS_I_FLAGS=-DDOCCOMMENTS_I_SOURCES
# endif

EXTRA_DIST = \
	$(TESTS) $(BUILT_SOURCES)

IMGSEEK_LIBS = $(builddir)/../../../xapian-extras/libxapian_imgseek.la
XAPIAN_PYTHON_LIBS = $(builddir)/../../../xapian-bindings/python/_xapian.la

pkgpylibdir = @PYTHON_LIB@/xapian/imgseek

# Install as _DATA rather than _SCRIPTS because we don't want to make these
# executable (they don't have a #! line).
pkgpylib_DATA = imgseek/__init__.py imgseek/__init__.pyc imgseek/__init__.pyo

pkgpylib_LTLIBRARIES = _imgseek.la

# Remove the .la file - _imgseek.la is never linked against (it's a module)
# and Python doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(pkgpylibdir)/_imgseek.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
	rm -f $(DESTDIR)$(pkgpylibdir)/_imgseek$(PYTHON_SO)

AM_CPPFLAGS = -I$(PYTHON_INC)
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS) -I$(srcdir)/../../../xapian-extras/imgseek/include
_imgseek_la_LDFLAGS = -avoid-version -module -shrext "$(PYTHON_SO)" $(NO_UNDEFINED)
_imgseek_la_SOURCES = imgseek_wrap.cc
_imgseek_la_LIBADD = $(XAPIAN_LIBS) $(IMGSEEK_LIBS) $(PYTHON_LIBS) $(XAPIAN_PYTHON_LIBS)

# In a distribution, imgseek.py is in $(srcdir), and python will always
# generate the .pyc and .pyo files in the same directory as the .py file.
# However, $(srcdir) might be shared between builds for different python
# versions so we can't let the .pyc and .pyo files get written there.  Hence we
# copy the .py file to the build directory from wherever it actually is and use
# the copy.  We also rename the copy to __init__.py so that it can be installed
# in a "imgseek" directory and be treated as a package.
imgseek/__init__.py: imgseek.py
	-test -d imgseek || mkdir imgseek
	cp `test -f imgseek.py || echo '$(srcdir)/'`imgseek.py imgseek/__init__.py

# We "import _imgseek" first so that if we fail to import the glue library
# we don't generate a broken __init__.pyc or __init__.pyo.
imgseek/__init__.pyc: imgseek/__init__.py imgseek/_imgseek$(PYTHON_SO)
	PYTHONPATH="imgseek:$$PYTHONPATH" $(PYTHON) -c "import _imgseek"
	PYTHONPATH=".:$(builddir)/../../../xapian-bindings/python/:$(builddir)/../../../xapian-bindings/python/xapian/:$$PYTHONPATH" $(PYTHON) -c "import imgseek"

imgseek/__init__.pyo: imgseek/__init__.py imgseek/_imgseek$(PYTHON_SO)
	PYTHONPATH="imgseek:$$PYTHONPATH" $(PYTHON) -O -c "import _imgseek"
	PYTHONPATH=".:$(builddir)/../../../xapian-bindings/python/:$(builddir)/../../../xapian-bindings/python/xapian/:$$PYTHONPATH" $(PYTHON) -O -c "import imgseek"

imgseek/_imgseek$(PYTHON_SO): _imgseek.la
	-test -d imgseek || mkdir imgseek
	$(LIBTOOL) --config > libtoolconfig.tmp
	. ./libtoolconfig.tmp; cp $$objdir/_imgseek$(PYTHON_SO) imgseek
	rm -f libtoolconfig.tmp

CLEANFILES = \
    imgseek/_imgseek$(PYTHON_SO) \
    imgseek/__init__.py imgseek/__init__.pyc imgseek/__init__.pyo \
    $(srcdir)/testsuite.pyc

# Clean the imgseek directory which we created, if it's empty, and any
# databases created by test cases.
clean-local:
	-rmdir imgseek
	rm -f db_test_* dbs_replication

if MAINTAINER_MODE
# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

BUILT_SOURCES += $(DOCCOMMENTS_I)
except.i: ../../../xapian-bindings/python/except.i
	echo cp "$^" "$@"

# imgseek_doccomments.i: doxy2swig.py ../../xapian-core/exception_data.pm ../../xapian-core/docs/apidoc/xml/index.xml
# 	$(PYTHON) doxy2swig.py ../../xapian-core/docs/apidoc/xml/index.xml imgseek_doccomments.i

imgseek_wrap.cc imgseek_wrap.h imgseek.py imgseek_wrap.d: imgseek_wrap.stamp
	stamp=imgseek_wrap.stamp; $(make_many_locked)
imgseek_wrap.stamp: $(SWIG_sources) $(DOCCOMMENTS_I)
	$(multitarget_begin)
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) $(DOCCOMMENTS_I_FLAGS) -c++ \
	    -python -threads -shadow -modern -O -outdir . \
	    -o imgseek_wrap.cc -MD -MF imgseek_wrap_d.tmp $(SWIG_mainsource)
	$(PERL) -pe 's!imgseek_wrap.cc:!imgseek_wrap.stamp:!' imgseek_wrap_d.tmp > imgseek_wrap_d2.tmp
	mv imgseek_wrap_d2.tmp imgseek_wrap.d
	rm imgseek_wrap_d.tmp
	$(PERL) -pe 's/class Error:/class Error(Exception):/' imgseek.py > imgseek_py.tmp
	mv imgseek_py.tmp imgseek.py
	$(multitarget_end)

-include imgseek_wrap.d

CLEANFILES += $(BUILT_SOURCES) imgseek_wrap.d imgseek_wrap.stamp
else
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
endif
