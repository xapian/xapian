## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i

ASSEMBLY_NAME=XapianSharp
ASSEMBLY=$(ASSEMBLY_NAME).dll
GACUTIL=gacutil
CSC=mcs

XAPIAN_SHARP_FILES=\
	$(srcdir)/*.cs

$(ASSEMBLY): $(XAPIAN_SHARP_FILES)
	$(CSC) --unsafe --target library -o $(ASSEMBLY) $(XAPIAN_SHARP_FILES)

BUILT_SOURCES = xapian_wrap.cc
xapian_wrap.cc: ../xapian.i util.i extra.i
	$(SWIG) -noproxy -module $(ASSEMBLY_NAME) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -csharp -o $@ $(srcdir)/../xapian.i

CLEANFILES = $(BUILT_SOURCES) *.cs
all-am: $(ASSEMBLY)
