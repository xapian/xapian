## Process this file with automake to produce Makefile.in

SWIG_INCLUDES          = $(INCLUDES) -I$(top_srcdir)/bindings/swig/
SWIG                   = swig
PERL_INTERNAL_INCLUDES = -I/usr/lib/perl5/5.005/i386-linux/CORE/

INCLUDES = -I$(top_srcdir)/include

target: omsee.so omsee.pm

# Special rules for generating files from SWIG

omstem_wrap.cc: ../swig/omstem.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -perl5 -shadow -o $@ $<
omsee_wrap.cc: ../swig/omsee.i ../swig/om_util.i ../swig/om_util_perl5.i ../swig/omtypes.i
	$(SWIG) $(SWIG_INCLUDES) -c++ -perl5 -shadow -o $@ $<

omstem.pm: omstem_wrap.cc
omsee.pm: omsee_wrap.cc

omstem_wrap.o: omstem_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PERL_INTERNAL_INCLUDES) -o $@
omsee_wrap.o: omsee_wrap.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< $(PERL_INTERNAL_INCLUDES) -o $@

omstem.so: omstem_wrap.o
	$(LIBTOOL) $(CXX) --mode=link -shared $< \
	$(top_builddir)/libomus.la -o $@
omsee.so: omsee_wrap.o
	$(LIBTOOL) $(CXX) --mode=link -shared $< \
	$(top_builddir)/libomus.la -o $@
