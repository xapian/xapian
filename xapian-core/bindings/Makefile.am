## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

if HAVE_SWIG
SWIG_SUBDIRS = python perl5 guile tcl8
else
SWIG_SUBDIRS =
endif

# FIXME: check for java
#JAVA_SUBDIRS = java
JAVA_SUBDIRS =

SUBDIRS = $(SWIG_SUBDIRS) $(JAVA_SUBDIRS)
DIST_SUBDIRS = python perl5 guile tcl8 java
