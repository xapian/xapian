## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

if HAVE_SWIG
SWIG_SUBDIRS = python perl5 guile tcl8
else
SWIG_SUBDIRS = 
endif

SUBDIRS = $(SWIG_SUBDIRS)
