INCLUDES = -I/usr/local/jdk/include -I/usr/local/jdk/include/linux

# directory to install libxapian_jni.{so,dynlib,dll,sl,...} in
jnidir = $(top_srcdir)/java/built
jni_LTLIBRARIES = libxapian_jni.la

libxapian_jni_la_SOURCES = Database.cc Document.cc ESet.cc ESetIterator.cc \
 Enquire.cc MSet.cc MSetIterator.cc PositionIterator.cc Query.cc RSet.cc \
 Stem.cc TermIterator.cc WritableDatabase.cc org_xapian_XapianJNI.cc utils.cc \
 org_xapian_XapianJNI.h xapian_jni.h XapianObjectHolder.h

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) -W -Wall -Wunused
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
