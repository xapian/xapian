## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i AssemblyInfo.cs.in
CLEANFILES = AssemblyInfo.cs $(ASSEMBLY).dll $(BUILT_SOURCES)

GACUTIL=gacutil
CSC=mcs

ASSEMBLY=XapianSharp

XAPIAN_SWIG_SRCS=\
	$(srcdir)/../xapian.i \
	$(srcdir)/util.i \
	$(srcdir)/extra.i

XAPIAN_SWIG_CS_SRCS=\
	$(srcdir)/SWIGTYPE_p_Xapian__Database.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__Document.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__ESet.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__ESetIterator.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__Enquire.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__ExpandDecider.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__MSet.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__MSetIterator.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__MatchDecider.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__PositionIterator.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__PostingIterator.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__Query.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__QueryParser.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__RSet.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__Stem.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__Stopper.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__TermIterator.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__ValueIterator.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__Weight.cs \
	$(srcdir)/SWIGTYPE_p_Xapian__WritableDatabase.cs \
	$(srcdir)/SWIGTYPE_p_std__listTstd__string_t.cs \
	$(srcdir)/SWIGTYPE_p_time_t.cs \
	$(srcdir)/SWIGTYPE_p_vectorTXapian__Query_p_t.cs \
	$(srcdir)/$(ASSEMBLY)PINVOKE.cs \
	$(srcdir)/$(ASSEMBLY).cs

$(ASSEMBLY).dll: $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs
	$(CSC) --unsafe --target library -o $(ASSEMBLY).dll $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs

BUILT_SOURCES = $(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS)
$(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS): $(XAPIAN_SWIG_SRCS)
	$(SWIG) -noproxy -module $(ASSEMBLY) -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -csharp -o $(ASSEMBLY).cc $(srcdir)/../xapian.i

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
