## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/include

SWIG_INCLUDES = $(INCLUDES) -I$(top_srcdir)/bindings/swig/

target: omuscatc.so omuscat.py

omstem_wrap.cc: ../swig/omstem.i ../swig/om_util.i
	swig $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $<

omuscat_wrap.cc: ../swig/omuscat.i ../swig/omstem.i ../swig/om_util.i ../swig/om_util_python.i
	swig $(SWIG_INCLUDES) -c++ -python -shadow -o $@ $<

omstem.py: omstem_wrap.cc

omuscat.py: omuscat_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/include/python1.5/ -o $@

omuscat_wrap.o: omuscat_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -I/usr/include/python1.5/ -o $@

omstemc.so: omstem_wrap.o $(top_builddir)/libomus.la
	$(CXX) -shared $< -o $@

omuscatc.so: omuscat_wrap.o
	$(LIBTOOL) $(CXX) -shared $< ../../libomus.la -o $@

apitestfuncs.py: $(top_srcdir)/tests/apitest.cc apitest_to_python.pl \
                 $(top_srcdir)/tests/apitest_parser.pm $(srcdir)/ToPython.pm
	@PERL_PATH@ -I$(srcdir)/ -I $(top_srcdir)/tests apitest_to_python.pl < $<

