## Process this file with automake to produce Makefile.in

# Note: we don't want dist tarballs built with docs missing, so for simplicity
# this Makefile.am blindly assumes the tools are in place when "make dist" is
# being run.  For "make all", "make check", etc we carefully skip any docs
# which we don't have the tools to generate.

# FIXME: add "xapian hacker docs" option to build sourcedoc.ps and HTML

SUFFIXES = .eps .dia .png

HTMLDOCS = index.html install.html intro_ir.html overview.html \
 stemming.html quickstart.html remote.html remote_protocol.html \
 quartzdesign.html matcherdesign.html tests.html scalability.html \
 bm25.html code_structure.html queryparser.html \
 quickstartexpand.cc.html quickstartindex.cc.html quickstartsearch.cc.html

# Files which should be put in the distribution by automake
EXTRA_DIST = mkdoc.pl \
	doxygen_api_conf.in doxygen_full_conf.in \
       	$(HTMLDOCS) \
	doxygen_api_header.html_tmpl \
	doxygen_api_footer.html_tmpl \
	doxygen_full_header.html_tmpl \
	doxygen_full_footer.html_tmpl \
	apidoc.ps

# Build API docs with doxygen
all-local: doxygen_api_docs

# Data to install, in the usual automake way
docdatadir   = $(datadir)/doc/@PACKAGE@
docdata_DATA = $(HTMLDOCS) apidoc.ps

dist-hook:
	: # Don't allow "make dist" if dot (part of graphviz) isn't installed
	test YES = "@DOXYGEN_HAVE_DOT@"
	$(mkinstalldirs) $(distdir)/apidoc
	if test -f apidoc/html/index.html ; then \
	  cp -a apidoc/* $(distdir)/apidoc/ ; \
	else \
	  cp -a $(srcdir)/apidoc/* $(distdir)/apidoc/ ; \
	fi

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdatadir)/apidoc/html
	if test -f apidoc/html/index.html ; then \
	  $(INSTALL_DATA) apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	else \
	  $(INSTALL_DATA) $(srcdir)/apidoc/html/* $(DESTDIR)$(docdatadir)/apidoc/html ; \
	fi

###############################
# How to clean up local files #
###############################

maintainer-clean-local:
	rm -rf apidoc sourcedoc

################################
# How to uninstall local files #
################################
uninstall-local:
	rm -rf $(DESTDIR)$(docdatadir)/apidoc

MAINTAINERCLEANFILES= doxygen_api_conf doxygen_full_conf \
	code_structure.html apidoc.ps sourcedoc.ps

.FORCE: doxygen_source_docs doxygen_api_docs doxygen_docs

doxygen_source_docs: sourcedoc/html/index.html
doxygen_api_docs: apidoc/html/index.html
doxygen_docs: doxygen_source_docs doxygen_api_docs

doxygen_api_conf: $(top_srcdir)/configure.in
doxygen_full_conf: $(top_srcdir)/configure.in

# BSD make won't VPATH except for implicit rules it seems, so we check for
# the situation where this matters and then use find -newer to check the
# dependencies ourselves!

# depend on configure.in so we rebuild if VERSION changes
code_structure.html: mkdoc.pl $(top_srcdir)/configure.in
	if test -f "$(srcdir)/code_structure.html" && \
	   test ! -f code_structure.html && \
	   test -z "`find $(srcdir)/mkdoc.pl $(top_srcdir)/configure.in -newer $(srcdir)/code_structure.html -print`" ; then \
	    echo Not rebuilding code_structure.html despite make with crude VPATH ;\
	else \
	    $(PERL) $(srcdir)/mkdoc.pl "$(VERSION)" $(top_srcdir) $@ ;\
	fi

# We run into problems with line length limits in older sed implementations
# (e.g. on Solaris) so use $T instead of $(top_srcdir) in APIDOC_SRC, etc
T=$(top_srcdir)

apidoc/html/index.html: $(top_srcdir)/configure.in \
		$(srcdir)/doxygen_api_header.html_tmpl \
		$(srcdir)/doxygen_api_footer.html_tmpl \
		$(srcdir)/doxygen_api_conf.in \
		$(APIDOC_SRC)
	if test -f "$(srcdir)/apidoc/html/index.html" && \
	   test ! -f "apidoc/html/index.html" && \
	   test -z "`find $(top_srcdir)/configure.in $(srcdir)/doxygen_api_header.html_tmpl $(srcdir)/doxygen_api_footer.html_tmpl $(srcdir)/doxygen_api_conf.in $(APIDOC_SRC) -newer $(srcdir)/apidoc/html/index.html -print`" ; then \
	    echo Not rebuilding apidoc/html/index.html despite make with crude VPATH ;\
	else \
	    rm -rf apidoc ;\
	    top_srcdir=$(top_srcdir) $(DOXYGEN) doxygen_api_conf ;\
	fi

apidoc.ps: apidoc/html/index.html
	if test -f "$(srcdir)/apidoc/html/index.html" && \
	   test ! -f "apidoc/html/index.html" && \
	   test -f "$(srcdir)/apidoc.ps" && test ! -f apidoc.ps && \
	   test -z "`find $(srcdir)/apidoc/html/index.html -newer $(srcdir)/apidoc.ps -print`" ; then \
	    echo Not rebuilding apidoc.ps despite make with crude VPATH ;\
	else \
	    cd apidoc/latex && make refman.ps && mv refman.ps ../../apidoc.ps ;\
	fi

sourcedoc/html/index.html: $(top_srcdir)/configure.in \
		$(srcdir)/doxygen_full_header.html_tmpl \
		$(srcdir)/doxygen_full_footer.html_tmpl \
		$(srcdir)/doxygen_full_conf.in \
		$(SOURCEDOC_CC_SRC) $(SOURCEDOC_C_SRC) $(SOURCEDOC_H_SRC) \
		$(APIDOC_SRC)
	if test -f "$(srcdir)/sourcedoc/html/index.html" && \
	   test ! -f "sourcedoc/html/index.html" && \
	   test -z "`find $(top_srcdir)/configure.in $(srcdir)/doxygen_full_header.html_tmpl $(srcdir)/doxygen_full_footer.html_tmpl $(srcdir)/doxygen_full_conf.in $(SOURCEDOC_CC_SRC) $(SOURCEDOC_C_SRC) $(SOURCEDOC_H_SRC) $(APIDOC_SRC) -newer $(srcdir)/sourcedoc/html/index.html -print`" ; then \
	    echo Not rebuilding sourcedoc/html/index.html despite make with crude VPATH ;\
	else \
	    rm -rf sourcedoc ;\
	    top_srcdir=$(top_srcdir) $(DOXYGEN) doxygen_full_conf ;\
	fi

sourcedoc.ps: sourcedoc/html/index.html
	if test -f "$(srcdir)/sourcedoc/html/index.html" && \
	   test ! -f "sourcedoc/html/index.html" && \
	   test -f "$(srcdir)/sourcedoc.ps" && test ! -f sourcedoc.ps && \
	   test -z "`find $(srcdir)/sourcedoc/html/index.html -newer $(srcdir)/sourcedoc.ps -print`" ; then \
	    echo Not rebuilding sourcedoc.ps despite make with crude VPATH ;\
	else \
	    cd sourcedoc/latex && make refman.ps && mv refman.ps ../../sourcedoc.ps ;\
	fi

# We used to do: $(LATEX) "\scrollmode\input" refman.tex
# doxygen does: latex refman.tex

##################################
# Generate images from dia files #
##################################

.dia.eps:
	dia -e $@ $<

.eps.png:
	pstoimg -aaliastext -antialias -center 0 -density 40 -depth 1 $<
