EXTRA_DIST +=\
		include/Makefile\
		include/xapian-letor/Makefile

xapianletorincludedir = $(incdir)/xapian-letor

inc_HEADERS =\
	include/xapian-letor.h

xapianletorinclude_HEADERS =\
		include/xapian-letor/featuremanager.h\
		include/xapian-letor/featurevector.h\
		include/xapian-letor/letor.h\
		include/xapian-letor/letor_features.h\
		include/xapian-letor/ranker.h\
		include/xapian-letor/scorer.h
