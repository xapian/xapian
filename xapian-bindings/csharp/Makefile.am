## Process this file with automake to produce Makefile.in

EXTRA_DIST = util.i extra.i

GACUTIL=gacutil
CSC=mcs

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
	$(srcdir)/XapianPINVOKE.cs \
	$(srcdir)/Xapian.cs

Xapian.dll: $(XAPIAN_SWIG_CS_SRCS)
	$(CSC) --unsafe --target library -o Xapian.dll $(XAPIAN_SWIG_CS_SRCS)

BUILT_SOURCES = xapian_wrap.cc $(XAPIAN_SWIG_CS_SRCS)
xapian_wrap.cc $(XAPIAN_SWIG_CS_SRCS): $(XAPIAN_SWIG_SRCS)
	$(SWIG) -noproxy -module Xapian -I/usr/include -I$(srcdir) $(XAPIAN_CXXFLAGS) -c++ -csharp -o $@ $(srcdir)/../xapian.i

lib_LTLIBRARIES = Xapian.la

Xapian_la_LDFLAGS = -avoid-version -module
nodist_Xapian_la_SOURCES = xapian_wrap.cc
Xapian_la_LIBADD = $(XAPIAN_LIBS)

install-data-local: Xapian.dll
	$(GACUTIL) /i Xapian.dll /f /package Xapian /gacdir $(libdir) /root $(DESTDIR)$(libdir)
uninstall-local:
	$(GACUTIL) /u Xapian /f /package Xapian /gacdir $(libdir) /root $(DESTDIR)$(libdir)

CLEANFILES = Xapian.dll $(BUILT_SOURCES)
all-am: Xapian.dll
