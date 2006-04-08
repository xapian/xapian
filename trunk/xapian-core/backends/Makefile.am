## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_builddir)/include -I$(top_srcdir)/include

noinst_HEADERS =

noinst_LTLIBRARIES = libbackend.la

libbackend_la_SOURCES = database.cc

# Define backend libraries to include.  To add a new one:
# i)   Add lines to configure.ac to define the automake conditional
#      "BUILD_BACKEND_NEWONE"
# ii)  Add lines below to define BUILDSUBDIR_NEWONE and LIBADD_NEWONE
# iii) Add $(BUILDSUBDIR_NEWONE) and $(LIBADD_NEWONE) to the definitions
#      of SUBDIRS and libbackend_la_LIBADD at the bottom of this file
# iv)  Write the backend library

if BUILD_BACKEND_MUSCAT36
BUILDSUBDIR_MUSCAT36 = muscat36
LIBADD_MUSCAT36 = muscat36/libmuscat36.la
endif

if BUILD_BACKEND_QUARTZ
BUILDSUBDIR_QUARTZ = quartz
LIBADD_QUARTZ = quartz/libquartz.la
endif

if BUILD_BACKEND_FLINT
BUILDSUBDIR_FLINT = flint
LIBADD_FLINT = flint/libflint.la
endif

# InMemoryPositionList is used by MapTermList which is used by Xapian::Document
# so disable inmemory more selectively.
BUILDSUBDIR_INMEMORY = inmemory
LIBADD_INMEMORY = inmemory/libinmemory.la

if BUILD_BACKEND_REMOTE
BUILDSUBDIR_REMOTE = net
LIBADD_REMOTE = net/libremote.la
endif

SUBDIRS = $(BUILDSUBDIR_MUSCAT36) \
          $(BUILDSUBDIR_QUARTZ) \
          $(BUILDSUBDIR_FLINT) \
          $(BUILDSUBDIR_INMEMORY) \
	  $(BUILDSUBDIR_REMOTE) multi .

DIST_SUBDIRS = muscat36 quartz flint inmemory net multi .

libbackend_la_LIBADD = \
          $(LIBADD_MUSCAT36) \
	  $(LIBADD_QUARTZ) \
	  $(LIBADD_FLINT) \
          $(LIBADD_INMEMORY) \
	  $(LIBADD_REMOTE) \
	  multi/libmulti.la
