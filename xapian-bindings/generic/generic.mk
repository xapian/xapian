## Makefile fragment included from each language binding directory which uses
## SWIG.  Any makefile rules or variables which should be set for all SWIG
## bindings should be placed here.

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

if NEED_INTREE_DYLD
# This is a hack for Mac OS X to work around "System Integrity Protection"
# which strips DYLD_* variables from the environment when running binaries
# in system bin directories.  This totally breaks running in-tree tests
# against system interpreters (e.g. /usr/bin/python), but even for an
# interpreter installed elsewhere we need to take special care because
# /bin/sh is also affected.  The trick we use is to define a variable
# which expands to the code needed to set DYLD_LIBRARY_PATH and inject
# this right when we run the interpreter.
export OSX_SIP_HACK_ENV=env DYLD_LIBRARY_PATH=$(INTREE_DYLD_PATH)
endif

# Define separately to allow overriding easily with: make SWIG_WERROR=
SWIG_WERROR = -Werror

# Recover from the removal of $@.  A full explanation of this is in the
# automake manual under the heading "Multiple Outputs".
make_many_locked = \
@if test -f $@; then :; else \
  trap 'rm -rf "$(stamp)-lck" "$(stamp)"' 1 2 13 15; \
  if mkdir '$(stamp)-lck' 2>/dev/null; then \
    rm -f '$(stamp)'; \
    $(MAKE) $(AM_MAKEFLAGS) '$(stamp)'; \
    result=$$?; rm -rf '$(stamp)-lck'; exit $$result; \
  else \
    while test -d '$(stamp)-lck'; do sleep 1; done; \
    test -f '$(stamp)'; \
  fi; \
fi

multitarget_begin = @rm -f $@-t; touch $@-t
multitarget_end = @mv -f $@-t $@
