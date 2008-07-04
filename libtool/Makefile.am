## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = 1.3e gnu

BUILD_SUBDIRS = . libltdl doc tests
CONF_SUBDIRS = @CONF_SUBDIRS@
SUBDIRS = $(BUILD_SUBDIRS)
DIST_SUBDIRS = $(BUILD_SUBDIRS) $(CONF_SUBDIRS)

# We need to export these variables
CFLAGS = @CFLAGS@
CPPLAGS = @CPPFLAGS@
LDFLAGS = @LDFLAGS@
LIBS= @LIBS@
FFLAGS = @FFLAGS@
FLIBS = @FLIBS@

aclocal_macros = libtool.m4 ltdl.m4

EXTRA_DIST = $(aclocal_macros) bootstrap libtoolize.in ltmain.in \
	mkstamp ChangeLog.0 ChangeLog.1
CLEANFILES = libtool libtoolize ltmain.shT

# These are required by libtoolize and must be executable when installed.
# Since _SCRIPTS gets the program transform applied we make them
# executable by hand
pkgdata_DATA = config.guess config.sub install-sh ltmain.sh

# This macro file should be visible to Automake's aclocal.
aclocal_DATA = $(aclocal_macros)

# The standalone libtool script, and the libtool distributor.
bin_SCRIPTS = libtool libtoolize

## These are installed as a subdirectory of pkgdatadir so that
## libtoolize --ltdl can find them later:
## Preserve order here so that generated files remain newer than
## their dependencies.
ltdldatadir = $(pkgdatadir)/libltdl
ltdldata_DATA = \
	libltdl/COPYING.LIB \
	libltdl/README \
	libltdl/configure.ac \
	libltdl/Makefile.am \
	libltdl/ltdl.c \
	libltdl/ltdl.h \
	libltdl/config.guess \
	libltdl/config.sub \
	libltdl/install-sh \
	libltdl/missing \
	libltdl/ltmain.sh \
	libltdl/acinclude.m4 \
	libltdl/aclocal.m4 \
	libltdl/Makefile.in \
	libltdl/configure \
	libltdl/config-h.in

libtool: $(srcdir)/ltmain.sh $(top_builddir)/configure.ac
	$(SHELL) $(top_builddir)/config.status --recheck
	chmod +x $@

libtoolize: $(srcdir)/libtoolize.in $(top_builddir)/config.status
	CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) $(top_builddir)/config.status
	chmod +x $@

$(srcdir)/ltmain.sh: $(srcdir)/ltmain.in $(TSDEPS)
	-rm -f ltmain.shT
	date=`$(SHELL) $(srcdir)/mkstamp < $(srcdir)/ChangeLog` && \
	sed -e 's/@''PACKAGE@/@PACKAGE@/' -e 's/@''VERSION@/@VERSION@/' \
	    -e "s%@""TIMESTAMP@%$$date%" $(srcdir)/ltmain.in > ltmain.shT
	mv -f ltmain.shT $@ || \
	(rm -f $@ && cp ltmain.shT $@ && rm -f ltmain.shT)
	cp $@ $(srcdir)/libltdl/

# All our rules should depend on these demo files.
all-recursive: $(srcdir)/libltdl/acinclude.m4 $(ACINCLUDE_M4_LIST)

@srcdir@/libltdl/acinclude.m4: $(srcdir)/libtool.m4 $(srcdir)/ltdl.m4
	-rm -f $@ $@T
	cat $(srcdir)/libtool.m4 $(srcdir)/ltdl.m4 > $@T
	mv $@T $@

@ACINCLUDE_M4_LIST@: $(srcdir)/libtool.m4
	-rm -f $@ $@T
	cat $(srcdir)/libtool.m4 > $@T
	mv $@T $@

.PHONY: configure-subdirs
configure-subdirs distdir: $(DIST_MAKEFILE_LIST)
@DIST_MAKEFILE_LIST@:
	dir=`echo $@ | sed 's,^[^/]*$$,.,;s,/[^/]*$$,,'`; \
	test -d $$dir || mkdir $$dir || exit 1; \
	abs_srcdir=`cd $(top_srcdir) && pwd`; \
	(cd $$dir && $$abs_srcdir/$$dir/configure) || exit 1

# Create and install libltdl
install-data-hook:
	chmod +x $(DESTDIR)$(pkgdatadir)/config.guess
	chmod +x $(DESTDIR)$(pkgdatadir)/config.sub
	chmod +x $(DESTDIR)$(pkgdatadir)/install-sh
	chmod +x $(DESTDIR)$(pkgdatadir)/libltdl/config.guess
	chmod +x $(DESTDIR)$(pkgdatadir)/libltdl/config.sub
	chmod +x $(DESTDIR)$(pkgdatadir)/libltdl/install-sh
	chmod +x $(DESTDIR)$(pkgdatadir)/libltdl/configure
	chmod +x $(DESTDIR)$(pkgdatadir)/libltdl/missing

# Uninstall libltdl
uninstall-local:
	-rm -rf $(DESTDIR)$(pkgdatadir)/libltdl


## ---------------------------------------------------------------- ##
##  Everything past here is useful to the maintainer, but probably  ##
##  not to anybody else (inspiration from automake/Makefile.am).    ##
## ---------------------------------------------------------------- ##

MAKEINFO = makeinfo

.PHONY: cvs-release
cvs-release: version-check prev-tarball cvs-news fetch cvs-commit cvs-dist deltas web-manual
	@tarname="$(PACKAGE)-$(VERSION).tar.gz"; \
	diffname="$(PACKAGE)-$(LASTRELEASE)-$(VERSION).diff.gz"; \
	xdeltaname="$(PACKAGE)-$(LASTRELEASE)-$(VERSION).xdelta"; \
	echo " *** Upload $$tarname, $$tarname.sig,";\
	echo " *** $$tarname.directive.asc, $$diffname,"; \
	echo " *** $$diffname.sig, $$diffname.directive.asc,"; \
	echo " *** $$xdeltaname, $$xdeltaname.sig and";\
	echo " *** $$xdeltaname.directive.asc to either"
	echo " *** /incoming/alpha or /incoming/ftp on ftp-upload.gnu.org."
	echo " *** You might need to upload manual.html to webcvs/libtool."

.PHONY: version-check
version-check:
	@case $(VERSION) in \
	*[acegikmoqsuwy]) \
	  echo "Version \`$(VERSION)' is not a releasable version, please read:"; \
	  echo "    http://www.gnu.org/software/libtool/contribute.html"; \
	  exit 1; \
	  ;; \
	esac

.PHONY: prev-tarball
prev-tarball:
## Make sure we have the previous release tarball in the tree.
	@if test -z "$(LASTRELEASE)"; \
		then echo "LASTRELEASE is not set"; exit 1; fi
	@ofile="$(PACKAGE)-$(LASTRELEASE).tar.gz"; \
	if test -f $$ofile; then :; \
	else echo "Cannot make deltas without $$ofile"; exit 1; fi

# TSDEPS will be defined to TSDEPS_DIST at `make dist' time
TSDEPS =
TSDEPS_DIST = ChangeLog libtool.m4
CVS = cvs # set it to `:' to avoid CVS operations

.PHONY: timestamps update-timestamps
timestamps: update-timestamps
update-timestamps:
	@if (cd $(srcdir) && test -d CVS && \
	    $(CVS) -n update $(TSDEPS_DIST) | grep '^M'); then \
		echo "Cannot make cvs-dist before commit"; exit 1; else :; fi


.PHONY: cvs-news
cvs-news:
## Make sure the NEWS file is up-to-date:
	@if sed '1,2d;3q' $(srcdir)/NEWS | grep -e "$(VERSION)" >/dev/null; \
	then :; \
	else \
	  echo "NEWS not updated; not releasing" 1>&2; \
	  exit 1; \
	fi

## Program to use to fetch files.
WGET = wget
WGETSGO = $(WGET) 'http://savannah.gnu.org/cgi-bin/viewcvs/~checkout~'
WGET_GIT = $(WGET) 'http://git.savannah.gnu.org/gitweb/?a=blob_plain;hb=HEAD;p='

## Files that we fetch and which we compare against.
## FIXME should be a lot more here
FETCHFILES = \
./INSTALL \
./config.guess \
libltdl/config.guess \
./config.sub \
libltdl/config.sub \
./install-sh \
libltdl/install-sh \
doc/texinfo.tex

## Fetch the latest versions of files we care about.
.PHONY: fetch
fetch:
	rm -rf Fetchdir > /dev/null 2>&1
	mkdir Fetchdir
## If a get fails then that is a problem.
	(cd Fetchdir && \
	$(WGET_GIT)'gnulib.git;f=doc/INSTALL' -O INSTALL && \
	$(WGET_GIT)'automake.git;f=lib/install-sh' -O install-sh && \
	$(WGETSGO)/config/config/config.guess -O config.guess && \
	$(WGETSGO)/config/config/config.sub -O config.sub && \
	$(WGETSGO)/texinfo/texinfo/doc/texinfo.tex -O texinfo.tex )
## Don't exit after test because we want to give as many errors as
## possible.
	@stat=0; for file in $(FETCHFILES); do \
	  fetchedfile=Fetchdir/`echo $$file | sed 's,.*/,,g'`; \
	  if diff -u $(srcdir)/$$file $$fetchedfile \
	    >>Fetchdir/update.patch 2>/dev/null; then :; \
	  else \
	    stat=1; \
	    echo "Updating $(srcdir)/$$file..."; \
	    cp $$fetchedfile $(srcdir)/$$file; \
	  fi; \
	done; \
	test $$stat = 1 && \
	  echo "See Fetchdir/update.patch for a log of the changes."; \
	exit $$stat


GPG = gpg # set it to `:' to avoid gpg operations

.PHONY: cvs-commit
cvs-commit: cvs-news
	cd $(srcdir) && $(SHELL) ./commit

.PHONY: cvs-dist
cvs-dist: cvs-news timestamps
## Build the distribution:
	$(MAKE) distcheck
## Finally, if everything was successful, tag the release
	cd $(srcdir) \
	  && $(CVS) -q tag -c `echo "release-$(VERSION)" | sed 's/\./-/g'`
## Generate signatures and directives for FSF ftp-upload:
	ofile="$(PACKAGE)-$(VERSION).tar.gz"; \
	$(GPG) --detach-sign $$ofile \
	&& echo "directory: libtool" > $$ofile.directive \
	&& $(GPG) --clearsign $$ofile.directive \
	&& rm -f $$ofile.directive

.PHONY: new-tarball
new-tarball:
## Make sure we have the new release tarball in the tree.
	@ofile="$(PACKAGE)-$(VERSION).tar.gz"; \
	if test -f $$ofile; then :; \
	else echo "Cannot make deltas without $$ofile"; exit 1; fi

.PHONY: got-xdelta
got-xdelta:
## Make sure xdelta exists;
	@if ($(XDELTA) --version 2>&1 | grep version)>/dev/null 2>/dev/null; \
	then :;\
	else \
	  echo "Get xdelta from http://sourceforge.net/projects/xdelta."; \
	  exit 1; \
	fi

.PHONY: deltas
deltas: delta-diff delta-xdelta

DIFF = diff
DIFF_OPTIONS = -ruNp

.PHONY: delta-diff
delta-diff: prev-tarball new-tarball
## Unpack the tarballs somewhere to diff them
	rm -rf delta-diff
	mkdir delta-diff
	ofile="../$(PACKAGE)-$(LASTRELEASE)-$(VERSION).diff.gz"; \
	cd delta-diff \
	&& tar xzf "../$(PACKAGE)-$(LASTRELEASE).tar.gz" \
	&& tar xzf "../$(PACKAGE)-$(VERSION).tar.gz" \
	&& $(DIFF) $(DIFF_OPTIONS) \
		$(PACKAGE)-$(LASTRELEASE) $(PACKAGE)-$(VERSION) \
		| GZIP=$(GZIP_ENV) gzip -c > $$ofile \
	&& $(GPG) --detach-sign $$ofile \
	&& echo "directory: libtool" > $$ofile.directive \
	&& $(GPG) --clearsign $$ofile.directive \
	&& rm -f $$ofile.directive
	rm -rf delta-diff

XDELTA = xdelta
XDELTA_OPTIONS = -9

.PHONY: delta-xdelta
delta-xdelta: prev-tarball new-tarball got-xdelta
## Generate the delta file (xdelta has wierd exit statuses, so we need to
## add some shell code to keep make happy), and then generate the signatures
## for FSF ftp-upload:
	ofile="$(PACKAGE)-$(LASTRELEASE)-$(VERSION).xdelta"; \
	( test -z `$(XDELTA) delta $(XDELTA_OPTIONS) \
	    $(PACKAGE)-$(LASTRELEASE).tar.gz $(PACKAGE)-$(VERSION).tar.gz \
	    $$ofile 2>&1` \
	  && : ) \
	&& $(GPG) --detach-sign $$ofile \
	&& echo "directory: libtool" > $$ofile.directive \
	&& $(GPG) --clearsign $$ofile.directive \
	&& rm -f $$ofile.directive

.PHONY: web-manual
web-manual:
	$(MAKEINFO) -I $(top_srcdir)/doc --html --no-split -o manual.html \
	$(top_srcdir)/doc/libtool.texi

