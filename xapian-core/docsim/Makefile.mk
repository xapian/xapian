noinst_HEADERS += docsim/docsim_internal.h

EXTRA_DIST += \
	docsim/dir_contents \
	docsim/Makefile

libxapian_la_SOURCES += \
	docsim/docsim.cc docsim/docsim_cosine.cc

