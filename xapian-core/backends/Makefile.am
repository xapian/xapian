## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS =

noinst_LTLIBRARIES = libbackend.la

libbackend_la_SOURCES = database_builder.cc database.cc

# Define backend libraries to include.  To add a new one:
# i)   Add lines to configure.in to define the automake conditional
#      "BUILD_BACKEND_NEWONE"
# ii)  Add lines below to define BUILDSUBDIR_NEWONE and LIBADD_NEWONE
# iii) Add $(BUILDSUBDIR_NEWONE) and $(LIBADD_NEWONE) to the definitions
#      of SUBDIRS and libbackend_la_LIBADD at the bottom of this file
# iv)  Write the library

if BUILD_BACKEND_MUSCAT36
BUILDSUBDIR_MUSCAT36 = muscat36
LIBADD_MUSCAT36 = muscat36/libmuscat36.la
endif

if BUILD_BACKEND_SLEEPYCAT
BUILDSUBDIR_SLEEPYCAT = sleepycat
LIBADD_SLEEPYCAT = sleepycat/libsleepycat.la
endif

if BUILD_BACKEND_QUARTZ
BUILDSUBDIR_QUARTZ = quartz
LIBADD_QUARTZ = quartz/libquartz.la
endif

if BUILD_BACKEND_INMEMORY
BUILDSUBDIR_INMEMORY = inmemory
LIBADD_INMEMORY = inmemory/libinmemory.la
endif

if BUILD_BACKEND_MULTI
BUILDSUBDIR_MULTI = multi
LIBADD_MULTI = multi/libmulti.la
endif

if BUILD_BACKEND_NET
BUILDSUBDIR_NET = net
LIBADD_NET = net/libnetdb.la
endif

SUBDIRS = \
          $(BUILDSUBDIR_MUSCAT36) \
	  $(BUILDSUBDIR_SLEEPYCAT) \
          $(BUILDSUBDIR_QUARTZ) \
          $(BUILDSUBDIR_INMEMORY) \
	  $(BUILDSUBDIR_MULTI) \
	  $(BUILDSUBDIR_NET) .
libbackend_la_LIBADD = \
          $(LIBADD_MUSCAT36) \
	  $(LIBADD_SLEEPYCAT) \
	  $(LIBADD_QUARTZ) \
          $(LIBADD_INMEMORY) \
	  $(LIBADD_MULTI) \
	  $(LIBADD_NET)
