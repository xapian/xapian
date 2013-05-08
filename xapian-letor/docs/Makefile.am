## Process this file with automake to produce Makefile.in

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped git tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

# Note: we don't want dist tarballs built with docs missing, so for simplicity
# this Makefile.am blindly assumes the tools are in place when "make dist" is
# being run.  For "make all", "make check", etc we carefully skip any docs
# which we don't have the tools to generate.

RSTDOCS = \
	letor.rst

RSTHTML = $(RSTDOCS:.rst=.html)

# FIXME: doxygen docs?
#
MAINTAINERCLEANFILES = \
	$(RSTHTML)

if DOCUMENTATION_RULES
.rst.html:
	$(RST2HTML) --exit-status=warning $< $@
endif
