noinst_HEADERS +=\
	weight/weightinternal.h

EXTRA_DIST +=\
	weight/collate-wdf-norm\
	weight/collate-idf-norm\
	weight/wdf-norm-dispatch.h\
	weight/idf-norm-dispatch.h\
	weight/Makefile

lib_src +=\
	weight/bb2weight.cc\
	weight/bm25plusweight.cc\
	weight/bm25weight.cc\
	weight/boolweight.cc\
	weight/coordweight.cc\
	weight/dicecoeffweight.cc\
	weight/dlhweight.cc\
	weight/dphweight.cc\
	weight/ifb2weight.cc\
	weight/ineb2weight.cc\
	weight/inl2weight.cc\
	weight/lmweight.cc\
	weight/pl2plusweight.cc\
	weight/pl2weight.cc\
	weight/tfidfweight.cc\
	weight/tradweight.cc\
	weight/weight.cc\
	weight/weightinternal.cc

weight/wdf-norm-dispatch.h: weight/collate-wdf-norm weight/Makefile.mk common/Tokeniseise.pm
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/weight/collate-wdf-norm' '$(srcdir)'

weight/idf-norm-dispatch.h: weight/collate-idf-norm weight/Makefile.mk common/Tokeniseise.pm
	$(PERL) -I'$(srcdir)/common' '$(srcdir)/weight/collate-idf-norm' '$(srcdir)'

BUILT_SOURCES +=\
	weight/wdf-norm-dispatch.h\
	weight/idf-norm-dispatch.h
