if VPATH_BUILD
# We need this so that generated sources can find non-generated headers in a
# VPATH build from SVN.
INCLUDES += -I$(top_srcdir)/letor
endif

noinst_HEADERS +=\
	letor/letor_internal.h

lib_src +=\
	letor/letor.cc\
	letor/letor_internal.cc
