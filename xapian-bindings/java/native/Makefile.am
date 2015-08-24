## Process this file with automake to produce Makefile.in

# `make QUIET=' overrides `./configure --enable-quiet'.
# `make QUIET=y' overrides `./configure' without `--enable-quiet'.
LIBTOOL = @LIBTOOL@ $(QUIET:y=--quiet)

if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

# directory to install libxapian_jni.{so,dll,sl,...} in
jnidir = `pwd`/../built
jni_LTLIBRARIES = libxapian_jni.la

# Remove the .la file - _xapian.la is never linked against (it's a module)
# and Java doesn't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(jnidir)/libxapian_jni.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the files ourselves.
uninstall-local:
if NEED_JNILIB_EXT
	rm -f $(DESTDIR)$(jnidir)/libxapian_jni.jnilib
else
	eval `grep '^dlname=' $(jni_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(jnidir)/"$$dlname"
endif

libxapian_jni_la_SOURCES = Database.cc Document.cc ESet.cc ESetIterator.cc \
 Enquire.cc MSet.cc MSetIterator.cc PositionIterator.cc Query.cc RSet.cc \
 Stem.cc TermIterator.cc WritableDatabase.cc org_xapian_XapianJNI.cc utils.cc \
 org_xapian_XapianJNI.h xapian_jni.h

if NEED_JNILIB_EXT
SHREXT=-shrext .jnilib
else
SHREXT=
endif
AM_CXXFLAGS = $(JAVA_CXXFLAGS) $(XAPIAN_CXXFLAGS)
AM_CPPFLAGS = $(JAVA_CPPFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(SHREXT) $(NO_UNDEFINED)
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
