INCLUDES += -I$(top_srcdir)/imgseek/include

# This path is for serialise-double.h
# FIXME - make xapian expose this properly.
INCLUDES += -I$(top_srcdir)/../xapian-core/common

noinst_HEADERS +=\
	imgseek/src/haar.h\
	imgseek/src/jpegloader.h

xapianinclude_HEADERS +=\
	imgseek/include/xapian/imgseek.h\
	imgseek/include/xapian/range_accelerator.h

libxapian_imgseek_la_SOURCES +=\
	imgseek/src/haar.cc\
	imgseek/src/image_terms.cc\
	imgseek/src/imgseek.cc\
	imgseek/src/jpegloader.cc\
	imgseek/src/range_accelerator.cc
