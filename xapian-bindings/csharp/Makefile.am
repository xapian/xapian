## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i AssemblyInfo.cs.in
CLEANFILES = AssemblyInfo.cs $(ASSEMBLY).dll $(BUILT_SOURCES)

ASSEMBLY=XapianSharp

XAPIAN_SWIG_SRCS=\
	$(srcdir)/../xapian.i \
	$(srcdir)/util.i \
	$(srcdir)/extra.i

XAPIAN_SWIG_CS_SRCS=\
	SWIGTYPE_p_Xapian__Database.cs \
	SWIGTYPE_p_Xapian__Document.cs \
	SWIGTYPE_p_Xapian__ESet.cs \
	SWIGTYPE_p_Xapian__ESetIterator.cs \
	SWIGTYPE_p_Xapian__Enquire.cs \
	SWIGTYPE_p_Xapian__ExpandDecider.cs \
	SWIGTYPE_p_Xapian__MSet.cs \
	SWIGTYPE_p_Xapian__MSetIterator.cs \
	SWIGTYPE_p_Xapian__MatchDecider.cs \
	SWIGTYPE_p_Xapian__PositionIterator.cs \
	SWIGTYPE_p_Xapian__PostingIterator.cs \
	SWIGTYPE_p_Xapian__Query.cs \
	SWIGTYPE_p_Xapian__QueryParser.cs \
	SWIGTYPE_p_Xapian__RSet.cs \
	SWIGTYPE_p_Xapian__Stem.cs \
	SWIGTYPE_p_Xapian__Stopper.cs \
	SWIGTYPE_p_Xapian__TermIterator.cs \
	SWIGTYPE_p_Xapian__ValueIterator.cs \
	SWIGTYPE_p_Xapian__Weight.cs \
	SWIGTYPE_p_Xapian__WritableDatabase.cs \
	SWIGTYPE_p_std__listTstd__string_t.cs \
	SWIGTYPE_p_time_t.cs \
	SWIGTYPE_p_vectorTXapian__Query_p_t.cs \
	op.cs \
	$(ASSEMBLY)PINVOKE.cs \
	$(ASSEMBLY).cs

XapianSharp.snk:
	$(SN) -k $@

$(ASSEMBLY).dll: $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs XapianSharp.snk
	$(CSC) --unsafe --target library -o $(ASSEMBLY).dll $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs

BUILT_SOURCES = $(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS)

# SWIG 1.3.21 and earlier don't generate op.cs.  An empty .cs file is allowed
# so we just touch it to avoid complicated contortions in the build system.
# Once we require SWIG 1.3.22 or newer, the touch can be removed.
$(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS): $(XAPIAN_SWIG_SRCS)
	$(SWIG) -noproxy -module $(ASSEMBLY) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -csharp -o $(ASSEMBLY).cc $(srcdir)/../xapian.i
	touch op.cs

lib_LTLIBRARIES = XapianSharp.la

AM_CXXFLAGS = $(XAPIAN_CXXFLAGS) @SWIG_CXXFLAGS@
XapianSharp_la_LDFLAGS = -avoid-version -module
nodist_XapianSharp_la_SOURCES = XapianSharp.cc
XapianSharp_la_LIBADD = $(XAPIAN_LIBS)

install-data-local: $(ASSEMBLY).dll
	$(GACUTIL) /i $(ASSEMBLY).dll /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)
uninstall-local:
	$(GACUTIL) /u $(ASSEMBLY) /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

all-am: $(ASSEMBLY).dll
