## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

## Test programs to be run
TESTS = smoketest.py pythontest.py
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)' ;\
	PYTHON3='$(PYTHON3)' ;\
	export abs_builddir ;\
	export srcdir ;\
	export PYTHON3 ;
LOG_COMPILER = '$(srcdir)'/run-python-test

installcheck-local:
	$(MAKE) check LOG_COMPILER='$(PYTHON3)' AM_TESTS_ENVIRONMENT= \
	    PYTHONDONTWRITEBYTECODE=1

RST_DOCS = \
	docs/examples.rst \
	docs/index.rst \
	docs/introduction.rst \
	docs/xapian.rst

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h xapian.py

EXTRA_DIST = python.i util.i extra.i extracomments.i except.i \
	doccomments.i \
	doxy2swig.py \
	fixup-swig-py3-wrapper \
	generate-python-exceptions \
	run-python-test \
	sphinx-build \
	testsuite.py \
	test_xapian_star.py \
	$(TESTS) $(BUILT_SOURCES) $(RST_DOCS)

pkgpylibdir = @PYTHON3_LIB@/xapian

# Install as _DATA rather than _SCRIPTS because we don't want to make these
# executable (they don't have a #! line).
pkgpylib_DATA = xapian/__init__.py \
		xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyc \
		xapian/__pycache__/__init__.@PYTHON3_CACHE_OPT1_EXT@

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
_xapian_la_SOURCES = xapian_wrap.cc
_xapian_la_LIBADD = $(XAPIAN_LIBS) $(PYTHON3_LIBS)

# We want `xapian/__init__.py` rather than xapian.py so the module is treated
# as a package.
xapian/__init__.py: xapian.py
	$(MKDIR_P) xapian
	cp `test -f xapian.py || echo '$(srcdir)/'`xapian.py xapian/__init__.py

# We "import _xapian" first so that if we fail to import the glue library
# we don't generate a broken .pyc or .pyo, and we do it with the current
# directory set to where the module is as Python with -c always searches the
# current directory first.
#
# We then "import xapian" as a separate command, as that's more like how things
# are once installed - in particular this will fail if the SWIG shadow wrapper
# doesn't cope with being xapian/__init__.py (currently it needs fixing
# up, which is done by fixup-swig-py3-wrapper).
xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyc: xapian/__init__.py xapian/_xapian$(PYTHON3_SO)
	$(OSX_SIP_HACK_ENV) $(PYTHON3) -c 'import os;os.chdir("xapian");import _xapian'
	$(OSX_SIP_HACK_ENV) $(PYTHON3) -c 'import xapian'

xapian/__pycache__/__init__.@PYTHON3_CACHE_OPT1_EXT@: xapian/__init__.py xapian/_xapian$(PYTHON3_SO)
	$(OSX_SIP_HACK_ENV) $(PYTHON3) -O -c 'import os;os.chdir("xapian");import _xapian'
	$(OSX_SIP_HACK_ENV) $(PYTHON3) -O -c 'import xapian'

xapian/_xapian$(PYTHON3_SO): _xapian.la
	$(MKDIR_P) xapian
	$(LIBTOOL) --config > libtoolconfig.tmp
## ksh requires a path on the sourced file.
	. ./libtoolconfig.tmp; cp $$objdir/_xapian$(PYTHON3_SO) xapian
	rm -f libtoolconfig.tmp

CLEANFILES = \
    xapian/_xapian$(PYTHON3_SO) \
    xapian/__init__.py \
    xapian/__pycache__/__init__.@PYTHON3_CACHE_TAG@.pyc \
    xapian/__pycache__/__init__.@PYTHON3_CACHE_OPT1_EXT@ \
    $(srcdir)/*.pyc

# Clean the xapian directory which we created and its __pycache__ subdirectory
# (if they're empty), output from sphinx, and any databases created by test
# cases.
clean-local:
	-rmdir xapian/__pycache__
	-rmdir xapian
	rm -rf doctrees docs/html db_test_* dbs_replication

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
	for a in pythontest smoketest testsuite ; do cp ../python/$${a}.py $${a}.py ; 2to3 -w --no-diffs $${a}.py ; done

# We need to explicitly set -outdir because on Windows, SWIG splits paths at
# "\" when extracting the output directory from the value passed to the -o
# option.

stamp = xapian_wrap.stamp
RUN_SWIG = stamp='$(stamp)' $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)
SWIG_FLAGS += -DSWIG_PYTHON_LEGACY_BOOL

xapian_wrap.cc xapian_wrap.h xapian.py xapian_wrap.d: $(stamp)
	$(make_many_locked)
$(stamp): except.i doccomments.i fixup-swig-py3-wrapper
	$(multitarget_begin)
	$(RUN_SWIG) $(SWIG_WERROR) -I. -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -python -threads -shadow -O \
	    -o xapian_wrap.cc '$(srcdir)/'python.i
	$(PERL) -i $(srcdir)/fixup-swig-py3-wrapper xapian.py
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += xapian_wrap.d $(stamp)
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

exampledatadir = $(docdir)/python3/examples
dist_exampledata_DATA = \
	docs/examples/simpleindex.py \
	docs/examples/simpleexpand.py \
	docs/examples/simplematchdecider.py \
	docs/examples/simplesearch.py

sphinxdocs = docs/html/index.html

install-data-local:
	$(mkinstalldirs) '$(DESTDIR)$(docdir)/python3'
	cp -R -p `test -r docs/html || echo '$(srcdir)/'`docs/html '$(DESTDIR)$(docdir)/python3'

all-local: $(sphinxdocs)

$(sphinxdocs): xapian/__init__.py xapian/_xapian$(PYTHON3_SO) docs/conf.py $(RST_DOCS) $(dist_exampledata_DATA)
	PYTHONPATH=..:$$PYTHONPATH $(OSX_SIP_HACK_ENV) $(PYTHON3) \
		$(srcdir)/sphinx-build \
		-b html -d doctrees -c docs $(srcdir)/docs docs/html
