# directory to install xapian_jni.{so,dynlib,dll,sl,...} in
jnidir = `pwd`/../built
jni_LTLIBRARIES = libxapian_jni.la

libxapian_jni_la_SOURCES = Database.cc Document.cc ESet.cc ESetIterator.cc \
 Enquire.cc MSet.cc MSetIterator.cc PositionIterator.cc Query.cc RSet.cc \
 Stem.cc TermIterator.cc WritableDatabase.cc org_xapian_XapianJNI.cc utils.cc \
 org_xapian_XapianJNI.h xapian_jni.h XapianObjectHolder.h

AM_CXXFLAGS = $(JAVA_CXXFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
