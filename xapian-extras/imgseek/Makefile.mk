INCLUDES += -I$(top_srcdir)/imgseek/include

noinst_HEADERS +=\
	imgseek/src/haar.h\
	imgseek/src/jpegloader.h

EXTRA_DIST +=\
	imgseek/dir_contents

libxapian_imgseek_la_SOURCES +=\
	imgseek/src/haar.cc\
	imgseek/src/imgseek.cc\
	imgseek/src/jpegloader.cc

