## Makefile fragment included from each language binding directory which uses
## SWIG.  Any makefile rules or variables which should be set for all SWIG
## bindings should be placed here.

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

SWIG_mainsource = \
	$(srcdir)/../xapian.i

SWIG_sources = \
	$(SWIG_mainsource) \
	$(srcdir)/../generic/except.i

SWIG_includes = \
	-I$(srcdir) \
	-I$(srcdir)/../generic

