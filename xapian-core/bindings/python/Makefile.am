## Process this file with automake to produce Makefile.in

PY_DESTDIR = @PYTHON_LIB@/site-packages/

INCLUDES = -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/extra
SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/
PY_CFLAGS = -I@PYTHON_INC@/

all: _xapian.so xapian.py

install:
	$(LIBTOOL) --mode=install $(INSTALL) _xapian.so $(PY_DESTDIR)
	$(INSTALL) xapian.py $(PY_DESTDIR)

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $(srcdir)/../swig/omstem.i

xapian_wrap.cc: ../swig/xapian.i ../swig/omstem.i ../swig/om_util.i ../swig/om_util_python.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $(srcdir)/../swig/xapian.i

omstem.py: omstem_wrap.cc

xapian.py: xapian_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PY_CFLAGS) -o $@

xapian_wrap.o: xapian_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PY_CFLAGS) -o $@

_omstem.so: omstem_wrap.o ../../libxapian.la
	$(LIBTOOL) --mode link $(CXX) -shared omstem_wrap.o ../../libxapian.la -o $@

_xapian.so: xapian_wrap.o ../../libxapian.la ../../extra/libomqueryparser.la
	$(LIBTOOL) --mode link $(CXX) -shared xapian_wrap.o ../../libxapian.la ../../extra/libomqueryparser.la -o $@

apitestfuncs.py: $(top_srcdir)/tests/apitest.cc apitest_to_python.pl \
                 $(top_srcdir)/tests/apitest_parser.pm $(srcdir)/ToPython.pm
	$(PERL) -I$(srcdir)/ -I $(top_srcdir)/tests apitest_to_python.pl < $(top_srcdir)/tests/apitest.cc

