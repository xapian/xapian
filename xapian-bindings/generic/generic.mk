## Makefile fragment included from each language binding directory which uses
## SWIG.  Any makefile rules or variables which should be set for all SWIG
## bindings should be placed here.

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@

if NEED_INTREE_DYLD
# This is a hack for Mac OS X to enable tests to work when built against an
# uninstalled xapian-core tree.  See https://trac.xapian.org/ticket/322
export DYLD_LIBRARY_PATH=$(INTREE_DYLD_PATH)
endif
endif

# Recover from the removal of $@.  A full explanation of this is in the
# automake manual under the heading "Multiple Outputs".
make_many_locked = \
if test -f $@; then :; else \
  trap 'rm -rf "$$stamp-lck" "$$stamp"' 1 2 13 15; \
  if mkdir "$$stamp-lck" 2>/dev/null; then \
    rm -f "$$stamp"; \
    $(MAKE) $(AM_MAKEFLAGS) "$$stamp"; \
    result=$$?; rm -rf "$$stamp-lck"; exit $$result; \
  else \
    while test -d "$$stamp-lck"; do sleep 1; done; \
    test -f "$$stamp"; \
  fi; \
fi

multitarget_begin = @rm -f $@-t; touch $@-t
multitarget_end = @mv -f $@-t $@

SWIG_mainsource = \
	$(srcdir)/../xapian.i

SWIG_sources = \
	$(SWIG_mainsource) \
	$(srcdir)/../generic/except.i

SWIG_includes = \
	-I$(srcdir) \
	-I$(srcdir)/../generic

