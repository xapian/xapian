## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign # don't fuss about GNU required files

INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include

noinst_HEADERS =

noinst_LTLIBRARIES = libbackend.la

libbackend_la_SOURCES = database_builder.cc

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

if BUILD_BACKEND_SLEEPY
BUILDSUBDIR_SLEEPY += sleepy
LIBADD_SLEEPY += sleepy/libsleepy.la
endif

if BUILD_BACKEND_INMEMORY
BUILDSUBDIR_INMEMORY += inmemory
LIBADD_INMEMORY += inmemory/libinmemory.la
endif

if BUILD_BACKEND_MULTI
BUILDSUBDIR_MULTI += multi
LIBADD_MULTI += multi/libmulti.la
endif

SUBDIRS = \
          $(BUILDSUBDIR_MUSCAT36) $(BUILDSUBDIR_SLEEPY) \
          $(BUILDSUBDIR_INMEMORY) $(BUILDSUBDIR_MULTI) .
libbackend_la_LIBADD = \
          $(LIBADD_MUSCAT36) $(LIBADD_SLEEPY) \
          $(LIBADD_INMEMORY) $(LIBADD_MULTI)
