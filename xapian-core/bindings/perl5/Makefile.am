## Process this file with automake to produce Makefile.in

SWIG_INCLUDES          = $(INCLUDES) -I$(top_srcdir)/bindings/swig/
SWIG                   = swig
PERL_INTERNAL_INCLUDES = -I/usr/lib/perl5/5.005/i386-linux/CORE/

INCLUDES = -I$(top_srcdir)/include

target: xapian.so xapian.pm

# Special rules for generating files from SWIG

omstem_wrap.cc: ../swig/omstem.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -perl5 -shadow -o $@ $<
xapian_wrap.cc: ../swig/xapian.i ../swig/om_util.i ../swig/om_util_perl5.i ../swig/omtypes.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -perl5 -shadow -o $@ $<

omstem.pm: omstem_wrap.cc
xapian.pm: xapian_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PERL_INTERNAL_INCLUDES) -o $@
xapian_wrap.o: xapian_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PERL_INTERNAL_INCLUDES) -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) --mode=link -shared $< \
	$(top_builddir)/libxapian.la -o $@
xapian.so: xapian_wrap.o
	$(LIBTOOL) $(CXX) --mode=link -shared $< \
	$(top_builddir)/libxapian.la -o $@
