## Process this file with automake to produce Makefile.in

INCLUDES = \
	$(GNOME_INCLUDEDIR) $(LIBOMUS_CFLAGS)

bin_PROGRAMS = indexergui

indexergui_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	display.cc graph.cc

indexergui_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) @LIBOMUS_LIBS@
