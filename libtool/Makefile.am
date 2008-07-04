## Makefile.am -- Process this file with automake to produce Makefile.in
##
##   Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008  Free Software
##   Foundation, Inc.
##   Written by Gary V. Vaughan, 2003
##
##   This file is part of GNU Libtool.
##
## GNU Libtool is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2 of
## the License, or (at your option) any later version.
##
## GNU Libtool is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with GNU Libtool; see the file COPYING.  If not, a copy
## can be downloaded from  http://www.gnu.org/licenses/gpl.html,
## or obtained by writing to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#####

ACLOCAL_AMFLAGS  	= -I libltdl/m4

AM_CPPFLAGS		=
AM_LDFLAGS		=

DIST_SUBDIRS		= .
EXTRA_DIST		=

BUILT_SOURCES		= libtool

CLEANFILES		=
MOSTLYCLEANFILES	=
DISTCLEANFILES		=

include_HEADERS		=
noinst_LTLIBRARIES	=
lib_LTLIBRARIES		=
EXTRA_LTLIBRARIES	=

auxdir			= libltdl/config
m4dir			= libltdl/m4

# Using `cd' in backquotes may print the directory name, use this instead:
lt__cd		= CDPATH="$${ZSH_VERSION+.}$(PATH_SEPARATOR)" && cd

MKSTAMP = $(SHELL) $(srcdir)/$(auxdir)/mkstamp

timestamp = set dummy `$(MKSTAMP) $(srcdir)`; shift; \
	case $(VERSION) in \
	  *[acegikmoqsuwy]) TIMESTAMP=" $$1 $$2" ;; \
	  *) TIMESTAMP="" ;; \
	esac

rebuild = rebuild=:; $(timestamp); correctver=$$1


## ---------------- ##
## Libtool scripts. ##
## ---------------- ##

# The libtool distributor and the standalone libtool script.
bin_SCRIPTS = libtoolize libtool

libtoolize: $(srcdir)/libtoolize.in $(top_builddir)/config.status
	rm -f libtoolize.tmp libtoolize
	$(timestamp); \
	input="libtoolize.m4sh"; \
	$(edit) -e "s,@TIMESTAMP\@,$$TIMESTAMP,g" \
		-e 's,@aclocal_DATA\@,$(aclocalfiles),g' \
		-e "s,@pkgltdl_files\@,$(ltdldatafiles),g" \
		-e "s,@pkgconfig_files\@,$(auxfiles),g" \
		$(srcdir)/libtoolize.in > libtoolize.tmp
	chmod a+x libtoolize.tmp
	chmod a-w libtoolize.tmp
	mv -f libtoolize.tmp libtoolize

# Use `$(srcdir)' for the benefit of non-GNU makes: this is
# how libtoolize.in appears in our dependencies.
EXTRA_DIST += libtoolize.m4sh
$(srcdir)/libtoolize.in: $(sh_files) libtoolize.m4sh Makefile.am
	cd $(srcdir); \
	rm -f libtoolize.in; \
	$(M4SH) -B $(auxdir) libtoolize.m4sh > libtoolize.in

# We used to do this with a 'stamp-vcl' file, but non-gmake builds
# would rerun configure on every invocation, so now we manually
# check the version numbers from the build rule when necessary.
libtool: $(top_builddir)/config.status $(srcdir)/$(auxdir)/ltmain.sh ChangeLog
	@target=libtool; $(rebuild); \
	if test -f "$$target"; then \
	  set dummy `./$$target --version | sed 1q`; actualver="$$5"; \
	  test "$$actualver" = "$$correctver" && rebuild=false; \
	fi; \
	for prereq in $?; do \
	  case $$prereq in *ChangeLog);; *) rebuild=:;; esac; \
	done; \
	if $$rebuild; then \
	  echo $(SHELL) ./config.status $$target; \
	  cd $(top_builddir) && $(SHELL) ./config.status $$target; \
	fi

.PHONY: configure-subdirs
configure-subdirs distdir: $(DIST_MAKEFILE_LIST)
@DIST_MAKEFILE_LIST@:
	dir=`echo $@ | sed 's,^[^/]*$$,.,;s,/[^/]*$$,,'`; \
	test -d $$dir || mkdir $$dir || exit 1; \
	abs_srcdir=`$(lt__cd) $(srcdir) && pwd`; \
	(cd $$dir && $$abs_srcdir/$$dir/configure --with-dist) || exit 1


# ---------- #
# Bootstrap. #
# ---------- #

edit = sed \
	-e 's,@EGREP\@,$(EGREP),g' \
	-e 's,@FGREP\@,$(FGREP),g' \
	-e 's,@GREP\@,$(GREP),g' \
	-e 's,@LN_S\@,$(LN_S),g' \
	-e 's,@MACRO_VERSION\@,$(VERSION),g' \
	-e 's,@PACKAGE\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_BUGREPORT\@,$(PACKAGE_BUGREPORT),g' \
	-e 's,@PACKAGE_NAME\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_STRING\@,$(PACKAGE) $(VERSION),g' \
	-e 's,@PACKAGE_TARNAME\@,$(PACKAGE),g' \
	-e 's,@PACKAGE_VERSION\@,$(VERSION),g' \
	-e 's,@SED\@,$(SED),g' \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@aclocaldir\@,$(aclocaldir),g' \
	-e 's,@datadir\@,$(datadir),g' \
	-e 's,@pkgdatadir\@,$(pkgdatadir),g' \
	-e 's,@host_triplet\@,$(host_triplet),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e "s,@configure_input\@,Generated from $$input.,g"

sh_files	= $(auxdir)/general.m4sh $(auxdir)/getopt.m4sh
EXTRA_DIST     += bootstrap $(srcdir)/libtoolize.in $(auxdir)/ltmain.m4sh \
		  $(auxdir)/mkstamp $(sh_files) \
		  ChangeLog.1996 ChangeLog.1997 ChangeLog.1998 \
		  ChangeLog.1999 ChangeLog.2000 ChangeLog.2001 \
		  ChangeLog.2002 ChangeLog.2003 ChangeLog.2004 \
		  ChangeLog.2005 ChangeLog.2006 ChangeLog.2007
CLEANFILES     += libtool libtoolize libtoolize.tmp \
		  $(auxdir)/ltmain.tmp $(m4dir)/ltversion.tmp

## We build ltversion.m4 here, instead of from config.status,
## because config.status is rerun each time one of configure's
## dependencies change and ltversion.m4 happens to be a configure
## dependency.  configure and ltversion.m4 would be rebuilt in
## a loop otherwise.
## We used to do this with a 'stamp-vcl' file, but non-gmake builds
## would rerun configure on every invocation, so now we manually
## check the version numbers from the build rule when necessary.
## Use `$(srcdir)/m4' for the benefit of non-GNU makes: this is
## how ltversion.m4 appears in our dependencies.
EXTRA_DIST += $(m4dir)/ltversion.in $(srcdir)/$(m4dir)/ltversion.m4
$(srcdir)/$(m4dir)/ltversion.m4: $(m4dir)/ltversion.in configure.ac ChangeLog
	@target='$(srcdir)/$(m4dir)/ltversion.m4'; $(rebuild); \
	if test -f "$$target"; then \
	  set dummy `sed -n '/^# serial /p' "$$target"`; shift; \
	  actualver=1.$$3; \
	  test "$$actualver" = "$$correctver" && rebuild=false; \
	fi; \
	for prereq in $?; do \
	  case $$prereq in *ChangeLog | *configure.ac);; *) rebuild=:;; esac; \
	done; \
	if $$rebuild; then \
	  cd $(srcdir); \
	  rm -f $(m4dir)/ltversion.tmp; \
	  serial=`echo "$$correctver" | sed 's,^1[.],,g'`; \
	  input="ltversion.in"; \
	  echo $(edit) -e "s,@MACRO_REVISION\@,$$correctver,g" \
	    -e "s,@MACRO_SERIAL\@,$$serial,g" \
	    $(srcdir)/$(m4dir)/ltversion.in \> $(srcdir)/$(m4dir)/ltversion.m4; \
	  $(edit) -e "s,@MACRO_REVISION\@,$$correctver,g" \
		  -e "s,@MACRO_SERIAL\@,$$serial,g" \
		  $(m4dir)/ltversion.in > $(m4dir)/ltversion.tmp; \
	  chmod a-w $(m4dir)/ltversion.tmp; \
	  mv -f $(m4dir)/ltversion.tmp $(m4dir)/ltversion.m4; \
	fi

## And for similar reasons, ltmain.sh can't be built from config.status.
## We used to do this with a 'stamp-vcl' file, but non-gmake builds
## would rerun configure on every invocation, so now we manually
## check the version numbers from the build rule when necessary.
## !WARNING! If you edit this rule to change the contents of ltmain.sh,
##           you must `touch $(srcdir)/$(auxdir)/ltmain.in' from the
##           shell if you need ltmain.sh to be regenerated.  Ideally, we
##           should make this rule depend on Makefile but that will break
##           distcheck (at least) by rebuilding ltmain.sh in the source
##           tree whenever config.status regenerates the Makefile.
EXTRA_DIST += $(srcdir)/$(auxdir)/ltmain.sh
$(srcdir)/$(auxdir)/ltmain.sh: $(sh_files) $(auxdir)/ltmain.m4sh configure.ac ChangeLog
	@target='$(srcdir)/$(auxdir)/ltmain.sh'; $(rebuild); \
	if test -f "$$target"; then \
	  eval `sed -n '/^package_revision=/p' "$$target"`; \
	  actualver=$$package_revision; \
	  test "$$actualver" = "$$correctver" && rebuild=false; \
	fi; \
	for prereq in $?; do \
	  case $$prereq in *ChangeLog);; *) rebuild=:;; esac; \
	done; \
	if $$rebuild; then \
	  cd $(srcdir); \
	  rm -f $(auxdir)/ltmain.in $(auxdir)/ltmain.tmp \
	    $(auxdir)/ltmain.sh; \
	  echo $(M4SH) -B $(auxdir) $(auxdir)/ltmain.m4sh \
	    \> $(auxdir)/ltmain.in; \
	  $(M4SH) -B $(auxdir) $(auxdir)/ltmain.m4sh \
	    > $(auxdir)/ltmain.in; \
	  input="ltmain.m4sh"; \
	  echo $(edit) -e "s,@TIMESTAMP\@,$$TIMESTAMP,g" \
	    -e "s,@package_revision\@,$$correctver," \
	    $(srcdir)/$(auxdir)/ltmain.in "> $$target"; \
	  $(edit) -e "s,@TIMESTAMP\@,$$TIMESTAMP,g" \
		-e "s,@package_revision\@,$$1,g" \
		$(auxdir)/ltmain.in > $(auxdir)/ltmain.tmp; \
	  rm -f $(auxdir)/ltmain.in; \
	  chmod a-w $(auxdir)/ltmain.tmp; \
	  mv -f $(auxdir)/ltmain.tmp $(auxdir)/ltmain.sh; \
	fi

$(srcdir)/libltdl/Makefile.am: $(srcdir)/libltdl/Makefile.inc
	cd $(srcdir); \
	in=libltdl/Makefile.inc; out=libltdl/Makefile.am; \
	rm -f $$out; \
	( $(SED) -n '1,/^.. DO NOT REMOVE THIS LINE -- /p' $$in; \
	{ echo 'ACLOCAL_AMFLAGS = -I m4'; \
	  echo 'AUTOMAKE_OPTIONS = foreign'; \
	  echo 'AM_CPPFLAGS ='; \
	  echo 'AM_LDFLAGS ='; \
	  echo 'BUILT_SOURCES ='; \
	  echo 'include_HEADERS ='; \
	  echo 'noinst_LTLIBRARIES ='; \
	  echo 'lib_LTLIBRARIES ='; \
	  echo 'EXTRA_LTLIBRARIES ='; \
	  echo 'EXTRA_DIST ='; \
	  echo 'CLEANFILES ='; \
	  echo 'MOSTLYCLEANFILES ='; \
	}; \
	$(SED) -n '/^.. DO NOT REMOVE THIS LINE -- /,$$p' $$in | \
	$(SED) -e 's,libltdl_,,; s,libltdl/,,; s,: libltdl/,: ,' \
	       -e 's,\$$(libltdl_,$$(,' \
	) | \
	$(SED) -e '/^.. DO NOT REMOVE THIS LINE -- /d' \
	       -e '1s,^\(.. Makefile.\)inc.*,\1am -- Process this file with automake to produce Makefile.in,' > $$out;
	chmod a-w $(srcdir)/libltdl/Makefile.am


## Unfortunately, all this bogeyness means that we have to manually
## keep the generated files in libltdl up to date.
all-local: $(srcdir)/libltdl/aclocal.m4 \
	$(srcdir)/libltdl/stamp-mk \
	$(srcdir)/libltdl/configure \
	$(srcdir)/libltdl/config-h.in

EXTRA_DIST += $(srcdir)/libltdl/stamp-mk $(m4dir)/lt~obsolete.m4

$(srcdir)/libltdl/Makefile.in: $(srcdir)/libltdl/Makefile.am \
	    $(srcdir)/libltdl/aclocal.m4 
	cd $(srcdir)/libltdl && $(AUTOMAKE) Makefile

$(srcdir)/libltdl/stamp-mk: $(srcdir)/libltdl/Makefile.in
	cd $(srcdir)/libltdl && \
	sed -e 's,config/mdate-sh,,' -e 's,config/texinfo.tex,,' \
	    -e 's,config/mkinstalldirs,,' \
	    < Makefile.in > Makefile.inT && \
	mv -f Makefile.inT Makefile.in
	echo stamp > $@

sub_aclocal_m4_deps = \
	$(srcdir)/libltdl/configure.ac \
	$(m4dir)/libtool.m4 \
	$(m4dir)/ltoptions.m4 \
	$(m4dir)/ltdl.m4 \
	$(srcdir)/$(m4dir)/ltversion.m4 \
	$(m4dir)/ltsugar.m4 \
	$(m4dir)/argz.m4 \
	$(m4dir)/lt~obsolete.m4

sub_configure_deps = $(sub_aclocal_m4_deps) $(srcdir)/libltdl/aclocal.m4

$(srcdir)/libltdl/aclocal.m4: $(sub_aclocal_m4_deps)
	cd $(srcdir)/libltdl && $(ACLOCAL) -I m4

$(srcdir)/libltdl/configure: $(sub_configure_deps)
	cd $(srcdir)/libltdl && $(AUTOCONF)

$(srcdir)/libltdl/config-h.in: $(sub_configure_deps)
	cd $(srcdir)/libltdl && $(AUTOHEADER)
	touch $@


## -------- ##
## Libltdl. ##
## -------- ##

include libltdl/Makefile.inc

if ! LTDL_SUBDIR_LIBOBJS
## workaround for Autoconf 2.59, Automake 1.9.6:
## we include these files twice, because of missing LIBOBJDIR support.
EXTRA_DIST += \
	argz.c libltdl/argz.c \
	lt__dirent.c libltdl/lt__dirent.c \
	lt__strl.c libltdl/lt__strl.c
endif


## -------------- ##
## Documentation. ##
## -------------- ##

info_TEXINFOS		= doc/libtool.texi
doc_libtool_TEXINFOS	= doc/PLATFORMS doc/fdl.texi doc/notes.texi
EXTRA_DIST	       += $(srcdir)/doc/notes.txt
## Workaround for a texinfo bug described here:
## http://thread.gmane.org/gmane.comp.tex.texinfo.bugs/3930
DISTCLEANFILES	       += libtool.dvi

all-local: $(srcdir)/doc/notes.txt

$(srcdir)/doc/notes.txt: $(srcdir)/doc/notes.texi
	cd $(srcdir)/doc && \
	  $(MAKEINFO) --no-headers $(MAKEINFOFLAGS) -o notes.txt notes.texi


## ------------- ##
## Installation. ##
## ------------- ##

# These are required by libtoolize and must be executable when installed.
# The timestamps on these files must be preserved carefully so we install,
# uninstall and set executable with custom rules here.
auxexefiles	= config/compile config/config.guess config/config.sub \
		  config/depcomp config/install-sh config/missing
auxfiles	= $(auxexefiles) config/ltmain.sh

# Everything that gets picked up by aclocal is automatically distributed,
# this is the list of macro files we install on the user's system.
aclocalfiles	= m4/argz.m4 m4/libtool.m4 m4/ltdl.m4 m4/ltoptions.m4 \
		  m4/ltsugar.m4 m4/ltversion.m4 m4/lt~obsolete.m4

## These are installed as a subdirectory of pkgdatadir so that
## libtoolize --ltdl can find them later:
ltdldatafiles	= libltdl/COPYING.LIB \
		  libltdl/README \
		  libltdl/Makefile.inc \
		  libltdl/Makefile.am \
		  libltdl/configure.ac \
		  libltdl/aclocal.m4 \
		  libltdl/Makefile.in \
		  libltdl/config-h.in \
		  libltdl/configure \
		  libltdl/argz_.h \
		  libltdl/argz.c \
		  libltdl/loaders/dld_link.c \
		  libltdl/loaders/dlopen.c \
		  libltdl/loaders/dyld.c \
		  libltdl/loaders/load_add_on.c \
		  libltdl/loaders/loadlibrary.c \
		  libltdl/loaders/shl_load.c \
		  libltdl/lt__dirent.c \
		  libltdl/lt__strl.c \
		  $(libltdl_libltdl_la_SOURCES)

install-data-local: libltdl/Makefile.in
	@$(NORMAL_INSTALL)
## Don't install over the top of an old pkgdatadir
	-rm -rf $(DESTDIR)$(pkgdatadir)/*
## To avoid spurious reconfiguration when the user installs these files
## with libtoolize, we have to install them in the correct order:
## First, put a copy of the libtool m4 macros in the aclocal dir
	$(mkinstalldirs) $(DESTDIR)$(aclocaldir)
	@list='$(aclocalfiles)'; for p in $$list; do \
	  f=`echo "$$p" | sed 's|^.*/||'`; \
	  echo " $(INSTALL_DATA) '$(srcdir)/$(m4dir)/$$f' '$(DESTDIR)$(aclocaldir)/$$f'"; \
	  $(INSTALL_DATA) "$(srcdir)/$(m4dir)/$$f" "$(DESTDIR)$(aclocaldir)/$$f"; \
	done
## install the helper scripts
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/config
	@list='$(auxfiles)' && for p in $$list; do \
	  echo " $(INSTALL_SCRIPT) '$(srcdir)/libltdl/$$p' '$(DESTDIR)$(pkgdatadir)/$$p'"; \
	  $(INSTALL_SCRIPT) "$(srcdir)/libltdl/$$p" "$(DESTDIR)$(pkgdatadir)/$$p"; \
	done
## install the libltdl files
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/libltdl
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/libltdl/libltdl
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/libltdl/loaders
	@list='$(ltdldatafiles)' && for p in $$list; do \
	  echo " $(INSTALL_DATA) '$(srcdir)/$$p' '$(DESTDIR)$(pkgdatadir)/$$p'"; \
	  $(INSTALL_DATA) "$(srcdir)/$$p" "$(DESTDIR)$(pkgdatadir)/$$p"; \
	done
	-chmod a+x $(DESTDIR)$(pkgdatadir)/libltdl/configure


## ------------- ##
## Distribution. ##
## ------------- ##

uninstall-hook:
	@$(NORMAL_UNINSTALL)
	@list='$(ltdldatafiles) $(auxfiles)'; for f in $$list; do \
	  echo " rm -f '$(DESTDIR)$(pkgdatadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
	@for p in $(aclocalfiles); do \
	  f=`echo "$$p" | sed 's|^.*/||'`; \
	  echo " rm -f '$(DESTDIR)$(aclocaldir)/$$f'"; \
	  rm -f "$(DESTDIR)$(aclocaldir)/$$f"; \
	done

dist-hook:
## Ship README.alpha only in alpha release, but renamed to README
	@if test -f $(srcdir)/README.alpha; then \
	case $(VERSION) in \
	  *[a-z]) cp -p $(srcdir)/README.alpha $(distdir)/README ;; \
	esac; else :; fi


## ----------- ##
## Test suite. ##
## ----------- ##

# The testsuite files are evaluated in the order given here.
TESTSUITE	= tests/testsuite
TESTSUITE_AT	= tests/testsuite.at \
		  tests/libtoolize.at \
		  tests/duplicate_members.at \
		  tests/duplicate_conv.at \
		  tests/duplicate_deps.at \
		  tests/inherited_flags.at \
		  tests/convenience.at \
		  tests/link-order.at \
		  tests/link-order2.at \
		  tests/fail.at \
		  tests/shlibpath.at \
		  tests/runpath-in-lalib.at \
		  tests/static.at \
		  tests/export.at \
		  tests/search-path.at \
		  tests/indirect_deps.at \
		  tests/archive-in-archive.at \
		  tests/execute-mode.at \
		  tests/localization.at \
		  tests/destdir.at \
		  tests/old-m4-iface.at \
		  tests/am-subdir.at \
		  tests/lt_dlexit.at \
		  tests/lt_dladvise.at \
		  tests/need_lib_prefix.at \
		  tests/standalone.at \
		  tests/subproject.at \
		  tests/nonrecursive.at \
		  tests/recursive.at \
		  tests/template.at \
		  tests/ctor.at \
		  tests/early-libtool.at \
		  tests/deplibs-ident.at \
		  tests/configure-iface.at \
		  tests/stresstest.at \
		  tests/cmdline_wrap.at \
		  tests/darwin.at

EXTRA_DIST     += $(srcdir)/$(TESTSUITE) $(TESTSUITE_AT) $(srcdir)/tests/package.m4

# Be sure to reexport important environment variables:
TESTS_ENVIRONMENT = MAKE="$(MAKE)" CC="$(CC)" CFLAGS="$(CFLAGS)" \
	CPP="$(CPP)" CPPFLAGS="$(CPPFLAGS)" LD="$(LD)" LDFLAGS="$(LDFLAGS)" \
	LIBS="$(LIBS)" LN_S="$(LN_S)" NM="$(NM)" RANLIB="$(RANLIB)" \
	STRIP="$(STRIP)" \
	OBJEXT="$(OBJEXT)" EXEEXT="$(EXEEXT)" \
	SHELL="$(SHELL)" CONFIG_SHELL="$(SHELL)" \
	CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" CXXCPP="$(CXXCPP)" \
	F77="$(F77)" FFLAGS="$(FFLAGS)" \
	FC="$(FC)" FCFLAGS="$(FCFLAGS)" \
	GCJ="$(GCJ)" GCJFLAGS="$(GCJFLAGS)"

BUILDCHECK_ENVIRONMENT = _lt_pkgdatadir="$(abs_top_srcdir)" \
	LIBTOOLIZE="$(abs_top_builddir)/libtoolize" \
	LIBTOOL="$(abs_top_builddir)/libtool" \
	tst_aclocaldir="$(abs_top_srcdir)/libltdl/m4"

INSTALLCHECK_ENVIRONMENT = \
	LIBTOOLIZE="$(bindir)/`echo libtoolize | sed '$(program_transform_name)'`" \
	LIBTOOL="$(bindir)/`echo libtool | sed '$(program_transform_name)'`" \
	LTDLINCL="-I$(includedir)" \
	LIBLTDL="$(libdir)/libltdl.la" \
	tst_aclocaldir="$(aclocaldir)"

# Use `$(srcdir)' for the benefit of non-GNU makes: this is
# how `testsuite' appears in our dependencies.
$(srcdir)/$(TESTSUITE): $(srcdir)/tests/package.m4 $(TESTSUITE_AT) Makefile.am
	cd $(srcdir)/tests && \
	$(AUTOTEST) `echo $(TESTSUITE_AT) | sed 's,tests/,,g'` -o testsuite.tmp && \
	mv -f testsuite.tmp testsuite

$(srcdir)/tests/package.m4: $(srcdir)/configure.ac
	{ \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} | $(edit) > $(srcdir)/tests/package.m4

tests/atconfig: $(top_builddir)/config.status
	$(SHELL) ./config.status tests/atconfig

DISTCLEANFILES += tests/atconfig

CD_TESTDIR	= abs_srcdir=`$(lt__cd) $(srcdir) && pwd`; cd tests

# Hook the test suite into the check rule
check-local: tests/atconfig $(srcdir)/$(TESTSUITE) libltdl/libltdlc.la $(bin_SCRIPTS)
	$(CD_TESTDIR); \
	CONFIG_SHELL="$(SHELL)" $(SHELL) $$abs_srcdir/$(TESTSUITE) \
	  $(TESTS_ENVIRONMENT) $(BUILDCHECK_ENVIRONMENT) $(TESTSUITEFLAGS)

# Run the test suite on the *installed* tree.
installcheck-local: tests/atconfig $(srcdir)/$(TESTSUITE)
	$(CD_TESTDIR); \
	CONFIG_SHELL="$(SHELL)" $(SHELL) $$abs_srcdir/$(TESTSUITE) \
	  $(TESTS_ENVIRONMENT) $(INSTALLCHECK_ENVIRONMENT) $(TESTSUITEFLAGS) \
	  AUTOTEST_PATH="$(exec_prefix)/bin"

# We need to remove any file droppings left behind by testsuite
clean-local: clean-local-legacy
	-$(CD_TESTDIR); \
	test -f $$abs_srcdir/$(TESTSUITE) && \
	    $(SHELL) $$abs_srcdir/$(TESTSUITE) --clean


## ------------- ##
## Legacy tests. ##
## ------------- ##

# !WARNING! Don't add any new tests here, we are migrating to an
#           Autotest driven framework, please add new test cases
#           using the new framework above.  When the migration is
#           complete this section should be removed.

CXX_TESTS = \
	tests/tagdemo-static.test tests/tagdemo-make.test \
	tests/tagdemo-exec.test tests/tagdemo-conf.test \
	tests/tagdemo-make.test tests/tagdemo-exec.test \
	tests/tagdemo-shared.test tests/tagdemo-make.test \
	tests/tagdemo-exec.test tests/tagdemo-undef.test \
	tests/tagdemo-make.test tests/tagdemo-exec.test

F77_TESTS = \
	tests/f77demo-static.test tests/f77demo-make.test \
	tests/f77demo-exec.test tests/f77demo-conf.test \
	tests/f77demo-make.test tests/f77demo-exec.test \
	tests/f77demo-shared.test tests/f77demo-make.test \
	tests/f77demo-exec.test

FC_TESTS = \
	tests/fcdemo-static.test tests/fcdemo-make.test \
	tests/fcdemo-exec.test tests/fcdemo-conf.test \
	tests/fcdemo-make.test tests/fcdemo-exec.test \
	tests/fcdemo-shared.test tests/fcdemo-make.test \
	tests/fcdemo-exec.test

COMMON_TESTS = \
	tests/link.test tests/link-2.test tests/nomode.test \
	tests/objectlist.test tests/quote.test tests/sh.test \
	tests/suffix.test tests/tagtrace.test \
	tests/cdemo-static.test tests/cdemo-make.test tests/cdemo-exec.test \
	tests/demo-static.test tests/demo-make.test tests/demo-exec.test \
	tests/demo-inst.test tests/demo-unst.test \
	tests/depdemo-static.test tests/depdemo-make.test \
	tests/depdemo-exec.test tests/depdemo-inst.test \
	tests/depdemo-unst.test \
	tests/mdemo-static.test tests/mdemo-make.test tests/mdemo-exec.test \
	tests/mdemo-inst.test tests/mdemo-unst.test \
	tests/cdemo-conf.test tests/cdemo-make.test tests/cdemo-exec.test \
	tests/demo-conf.test tests/demo-make.test tests/demo-exec.test \
	tests/demo-inst.test tests/demo-unst.test tests/demo-deplibs.test \
	tests/depdemo-conf.test tests/depdemo-make.test \
	tests/depdemo-exec.test tests/depdemo-inst.test \
	tests/depdemo-unst.test \
	tests/mdemo-conf.test tests/mdemo-make.test tests/mdemo-exec.test \
	tests/mdemo-inst.test tests/mdemo-unst.test tests/mdemo-dryrun.test \
	tests/mdemo2-conf.test tests/mdemo2-make.test tests/mdemo2-exec.test \
	tests/pdemo-conf.test tests/pdemo-make.test tests/pdemo-exec.test \
	tests/pdemo-inst.test \
	tests/demo-nofast.test tests/demo-make.test tests/demo-exec.test \
	tests/demo-inst.test tests/demo-unst.test \
	tests/depdemo-nofast.test tests/depdemo-make.test \
	tests/depdemo-exec.test tests/depdemo-inst.test \
	tests/depdemo-unst.test \
	tests/demo-pic.test tests/demo-make.test tests/demo-exec.test \
	tests/demo-nopic.test tests/demo-make.test tests/demo-exec.test \
	tests/cdemo-shared.test tests/cdemo-make.test tests/cdemo-exec.test \
	tests/demo-shared.test tests/demo-make.test tests/demo-exec.test \
	tests/demo-inst.test tests/demo-hardcode.test tests/demo-relink.test \
	tests/demo-noinst-link.test tests/demo-unst.test \
	tests/depdemo-shared.test tests/depdemo-make.test \
	tests/depdemo-exec.test tests/depdemo-inst.test \
	tests/depdemo-relink.test tests/depdemo-unst.test \
	tests/mdemo-shared.test tests/mdemo-make.test tests/mdemo-exec.test \
	tests/mdemo-inst.test tests/mdemo-unst.test \
	tests/cdemo-undef.test tests/cdemo-make.test tests/cdemo-exec.test

TESTS = $(COMMON_TESTS)
if HAVE_CXX
TESTS += $(CXX_TESTS)
endif
if HAVE_F77
TESTS += $(F77_TESTS)
endif
if HAVE_FC
TESTS += $(FC_TESTS)
endif

tests/demo-conf.test: libtool

EXTRA_DIST     += $(srcdir)/tests/defs.in tests/defs.m4sh \
		  $(COMMON_TESTS) $(CXX_TESTS) $(F77_TESTS) $(FC_TESTS)
DIST_SUBDIRS   += $(CONF_SUBDIRS)

# The defs script shouldn't be recreated whenever the Makefile is
# regenerated since the source tree can be read-only.
check-recursive: tests/defs
tests/defs: $(srcdir)/tests/defs.in
	rm -f tests/defs.tmp tests/defs; \
	input="defs.m4sh"; \
	$(edit) $(srcdir)/tests/defs.in > tests/defs.tmp; \
	mv -f tests/defs.tmp tests/defs

# Use `$(srcdir)/tests' for the benefit of non-GNU makes: this is
# how defs.in appears in our dependencies.
$(srcdir)/tests/defs.in: $(auxdir)/general.m4sh tests/defs.m4sh Makefile.am
	cd $(srcdir); \
	rm -f tests/defs.in; \
	$(M4SH) -B $(auxdir) tests/defs.m4sh > tests/defs.in

# We need to remove any files that the above tests created.
clean-local-legacy:
	-cd tests; \
	for dir in cdemo demo depdemo f77demo fcdemo mdemo mdemo2 pdemo tagdemo; \
	do \
	    test -f $$dir/Makefile && ( cd $$dir && $(MAKE) distclean; ); \
	done
	rm -rf _inst

$(TESTS): tests/defs
DISTCLEANFILES += tests/defs
