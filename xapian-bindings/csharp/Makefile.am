## Process this file with automake to produce Makefile.in

TESTS_ENVIRONMENT =

## Test programs to be run
TESTS = SmokeTest.exe

EXTRA_DIST = util.i extra.i AssemblyInfo.cs.in SmokeTest.cs

ASSEMBLY=XapianSharp

XAPIAN_SWIG_SRCS=\
	$(srcdir)/../xapian.i \
	$(srcdir)/util.i \
	$(srcdir)/extra.i

XAPIAN_SWIG_CS_SRCS=\
	Database.cs \
	Document.cs \
	ESet.cs \
	ESetIterator.cs \
	Enquire.cs \
	MSet.cs \
	MSetIterator.cs \
	PositionIterator.cs \
	PostingIterator.cs \
	Query.cs \
	QueryParser.cs \
	RSet.cs \
	SWIGTYPE_p_Xapian__ExpandDecider.cs \
	SWIGTYPE_p_Xapian__MatchDecider.cs \
	SWIGTYPE_p_Xapian__Stopper.cs \
	SWIGTYPE_p_Xapian__Weight.cs \
	SWIGTYPE_p_std__listTstd__string_t.cs \
	SWIGTYPE_p_std__vectorTstd__string_t.cs \
	SWIGTYPE_p_time_t.cs \
	Stem.cs \
	TermIterator.cs \
	ValueIterator.cs \
	WritableDatabase.cs \
	Xapian.cs \
	XapianPINVOKE.cs \
	op.cs

XapianSharp.snk:
	$(SN) -k $@

$(ASSEMBLY).dll: $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs XapianSharp.snk
	$(CSC) --unsafe --target library -o $(ASSEMBLY).dll $(XAPIAN_SWIG_CS_SRCS) AssemblyInfo.cs

BUILT_SOURCES = $(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS)

# SWIG 1.3.21 and earlier don't generate op.cs.  An empty .cs file is allowed
# so we just touch it to avoid complicated contortions in the build system.
# Once we require SWIG 1.3.22 or newer, the touch can be removed.
$(ASSEMBLY).cc $(XAPIAN_SWIG_CS_SRCS): $(XAPIAN_SWIG_SRCS)
	$(SWIG) -I$(srcdir) @SWIG_FLAGS@ -c++ -csharp -namespace Xapian -module Xapian -o $(ASSEMBLY).cc $(srcdir)/../xapian.i
	touch op.cs

lib_LTLIBRARIES = XapianSharp.la

AM_CXXFLAGS = @SWIG_CXXFLAGS@
XapianSharp_la_LDFLAGS = -avoid-version -module
nodist_XapianSharp_la_SOURCES = XapianSharp.cc
XapianSharp_la_LIBADD = $(XAPIAN_LIBS)

install-data-local: $(ASSEMBLY).dll
	$(GACUTIL) /i $(ASSEMBLY).dll /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

uninstall-local:
	$(GACUTIL) /u $(ASSEMBLY) /f /package $(ASSEMBLY) /gacdir $(libdir) /root $(DESTDIR)$(libdir)

SmokeTest.exe: SmokeTest.cs $(ASSEMBLY).dll
	$(CSC) --unsafe --target exe SmokeTest.cs -r:$(ASSEMBLY).dll

all-am: $(ASSEMBLY).dll

CLEANFILES = $(BUILT_SOURCES) \
  XapianSharp.snk \
  XapianSharp.loT \
  AssemblyInfo.cs \
  $(ASSEMBLY).dll \
  SmokeTest.exe
