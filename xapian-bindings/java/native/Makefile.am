## Process this file with automake to produce Makefile.in

if OVERRIDE_MACOSX_DEPLOYMENT_TARGET
# This requires GNU make, but apparently that's the default on OS X.
export MACOSX_DEPLOYMENT_TARGET=@OVERRIDE_MACOSX_DEPLOYMENT_TARGET@
endif

# directory to install libxapian_jni.{so,dll,sl,...} in
jnidir = `pwd`/../built
jni_LTLIBRARIES = libxapian_jni.la

install-data-hook:
if NEED_JNILIB_EXT
	mv $(DESTDIR)$(jnidir)/libxapian_jni.so $(DESTDIR)$(jnidir)/libxapian_jni.jnilib
endif

libxapian_jni_la_SOURCES = Database.cc Document.cc ESet.cc ESetIterator.cc \
 Enquire.cc MSet.cc MSetIterator.cc PositionIterator.cc Query.cc RSet.cc \
 Stem.cc TermIterator.cc WritableDatabase.cc org_xapian_XapianJNI.cc utils.cc \
 org_xapian_XapianJNI.h xapian_jni.h XapianObjectHolder.h

AM_CXXFLAGS = $(JAVA_CXXFLAGS) $(XAPIAN_CXXFLAGS)
AM_CPPFLAGS = $(JAVA_CPPFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
